/**

Grundlegende Idee

Es wird eine Subsumption Vorgehensweise implementiert. Im Hintergrund läuft
einer cooperativer Scheduler 

Subsumption
Wiederkehrende Funktionen werden als TASKS definiert.
Alle tasks werden in einer "arbitrate-Funktion" ausgewertet und priorisiert

Im Prinzip wird die Ausgabe eines geringer priorisierten Tasks durch den nächst
höheren Task überschrieben

Beispiel
Task1 = CRUISE (der PodRacer fliegt einfach geradeaus und hält die Höhe)
Task2 = MovementControl (kontrolliert den OpticalFlowSensor)
Task3 = SurfaceDistance1 (kontrolliert die Höhe über Grund mit dem TFMiniLidar)
Task4 = SurfaceDistance2 (kontrolliert die Höhe über Grund mit dem VL53L1)
Task5 = Receiver - kontrolliert den Empfang von Daten und die Weiterleitung an den FC

Arbitrate (Priorisierung)


**/

// Include CoopTask since we want to manage multiple tasks.
#include <CoopTaskBase.h>
#include <CoopTask.h>
#include <CoopSemaphore.h>
#include <CoopMutex.h>
#include <BasicCoopTask.h>
#include <assert.h>
#include "Receiver.h"
#include "Arbitrate.h"
#include "SimpleLog.cpp"
#include "constants.h"


CoopSemaphore taskSema(1,1);
#define NUMBER_OF_LAYER_TASKS 5
int taskToken = 1;

SLog logger(&Serial, 115200, LOGLEVEL);

CoopTask<void>* heartBeatTask = nullptr;
//CoopTask<void>* movementCtrlTask = nullptr;
//CoopTask<void>* surfaceDist1Task = nullptr;
//CoopTask<void>* surfaceDist2Task = nullptr;
CoopTask<void>* receiverTask = nullptr;
CoopTask<void>* arbitrateTask = nullptr;


recv::Receiver receiver(&logger, &Serial2, 16, 17, true);
arbi::Arbitrate arbitrate(&logger);


//Receiver *receiver;

// Task 1 : HeartBeat (1 second)
void HeartBeatFunction() {
//  taskSema.wait();
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);
  for(;;) {
    //logger.info("blink");
    yield();
    digitalWrite(LED_BUILTIN, LOW);
    delay(HB_BLINK_FREQ);
    digitalWrite(LED_BUILTIN, HIGH);
    delay(HB_BLINK_FREQ);
    //CoopTask<void>::sleep();
    yield();
  }
}

/*
// Task 2: use PMW3901 optical flow sensor
void MovementControlFunction() {
//  taskSema.wait();
  unsigned long lastMillis = millis();
  int16_t deltaX,deltaY;

  for(;;) {
    Serial.println("run MovementControlFunction");
    
    //flow.readMotionCount(&deltaX, &deltaY);
    if ((millis() - lastMillis) > LOOP_TIME) {
      yield();
    }
    delay(LOOP_TIME);
  }

}

// Task 3: use TFMini sensor
void SurfaceDistanceControl1Function() {
  unsigned long lastMillis = millis();
  for(;;) {
    //flow.readMotionCount(&deltaX, &deltaY);
    if ((millis() - lastMillis) > LOOP_TIME) {
      yield();
    }
    delay(LOOP_TIME);
  }

}

// Task 4: use VL53L01 sensor
void SurfaceDistanceControl2Function() {
  unsigned long lastMillis = millis();
  for(;;) {
    Serial.println("run SurfaceDistanceControl2Function");

    //flow.readMotionCount(&deltaX, &deltaY);
    if ((millis() - lastMillis) > LOOP_TIME) {
      yield();
    }
    delay(LOOP_TIME);  
  }

}
*/

// Task 5: control SBUS activites (Receiver -> EPS32 -> FlightController)
void ReceiverFunction() {
  unsigned long lastMillis = millis();
  if (!receiver.begin()) {
    logger.error("MAIN : can't create Receiver object");
    return;
  }
  for(;;) {
    /** read latest hardware-receiver data into SDATA struct **/
    receiver.update();
    if ((millis() - lastMillis) > LOOP_TIME) {
      lastMillis = millis();
      yield();
    }
    delay(LOOP_TIME);
  }

}
// Task 6: arbitrate 
void ArbitrateFunction() {
  unsigned long lastMillis = millis();
  if (&receiver) {
    if (!arbitrate.begin(&receiver)) {
      logger.error("MAIN : can't create arbitrate object");
    }
  }
  else {
    logger.error("arbitrate.begin() - no receiver object");
  }
  for(;;) {

    arbitrate.update();


    //flow.readMotionCount(&deltaX, &deltaY);
    if ((millis() - lastMillis) > LOOP_TIME) {
      yield();
    }
    delay(LOOP_TIME);
  }
  
}

bool DelayCoopFunction(int ms) {
  return true;
}

bool SleepCoopFunction() {
  Serial.println("run SleepCoopFunction");
  Serial.flush();
  delay(LOOP_TIME);
  Serial.print(">> hBT:"); Serial.println((unsigned long)&heartBeatTask);

  if (heartBeatTask) {
    logger.info(">>> Wakup HB");
    heartBeatTask->wakeup();
  }
  else {
    logger.info("!!!! no obj HB");
  }
  //if (movementCtrlTask) movementCtrlTask->wakeup();
  //if (surfaceDist1Task) surfaceDist1Task->wakeup();
  //if (surfaceDist2Task) surfaceDist2Task->wakeup();
  if (receiverTask) receiverTask->wakeup();
  if (arbitrateTask) arbitrateTask->wakeup();

  return true;
}


void setup() {
  while (!Serial);
  delay(100);

  #if !defined(ESP32)
    Serial.println("!!!!!!!!!!!!!!!!!!!!!!!!!!!");
    Serial.println("not running on ESP32 device");
    Serial.println("!!!!!!!!!!!!!!!!!!!!!!!!!!!");
    assert(0);
  #endif

  //Serial.println("setup.....");
  logger.info("setup...");

  //-----------------------------------------
  logger.info("create tasks....");

  heartBeatTask = new CoopTask<void>(F("HeartBeat"),HeartBeatFunction);
  //movementCtrlTask = new CoopTask<void>(F("MovementCtrl"), MovementControlFunction);
  //surfaceDist1Task= new CoopTask<void>(F("SurfaceDist 1"), SurfaceDistanceControl1Function);
  //surfaceDist2Task = new CoopTask<void>(F("SurfaceDist 2"), SurfaceDistanceControl2Function);
  receiverTask = new CoopTask<void>(F("RECEIVER"),ReceiverFunction);
  arbitrateTask = new CoopTask<void>(F("ARBITRATE"),ArbitrateFunction);
  logger.info("all tasks ready");

  //-----------------------------------------
  logger.info("schedule tasks");

Serial.print("heartBeatTask:"); Serial.println((unsigned long)&heartBeatTask);
  runCoopTasks(nullptr, nullptr, SleepCoopFunction);


  heartBeatTask->scheduleTask();
  //movementCtrlTask->scheduleTask();
  //surfaceDist1Task->scheduleTask();
  //surfaceDist2Task->scheduleTask();
  receiverTask->scheduleTask();
  arbitrateTask->scheduleTask();
  logger.info("all tasks scheduled");
}



void loop() {
  // put your main code here, to run repeatedly:
  //runCoopTasks(nullptr, DelayCoopFunction, SleepCoopFunction);
  runCoopTasks(nullptr, nullptr, nullptr);
  //Serial.println("running tasks....");
  yield();
}
