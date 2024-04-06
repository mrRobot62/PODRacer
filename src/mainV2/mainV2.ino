/***********************************************************************************************



***********************************************************************************************/

// PODRacer use CooperativeTask mechanism
#include <CoopTaskBase.h>
#include <CoopTask.h>
#include <CoopSemaphore.h>
#include <CoopMutex.h>
#include <BasicCoopTask.h>
#include <assert.h>
#include "global.h"
#include "global_utils.h"

// PODRacer Classes/Stuff
#include "Mixer.h"              // this is not a task; responsible to calculate new writings for receiver, based on all below tasks
#include "Receiver.h"           // this is not a task; responsible for reading/writing SBUS commands
#include "Logger.h"             // this is not a task; our logger class
#include "Blackbox.h"           // this is not a task; responsible to save data (blackbox) on SDCard
#include "WiFiData.h"           // this is not a task; responsible to save data (blackbox) on SDCard
#include "TaskBlink.h"          // task; responsible to generate blink-patterns
#include "TaskOpticalFlow.h"    // task; responsible to calculate new data based on the optical flow sensor
#include "TaskSurface.h"        // task; responsible to calculate new data based on both distance sensors
#include "TaskSteering.h"       // task; responsible to calculate a smooth steering
#include "TaskHover.h"          // task; responsible to calculate the hight of the PODRacer (hovering)

char * _tname = "MAIN";         // used to log MAIN output

//HardwareSerial hsBus1(1);       // used by LIDAR Sensor in TaskSurface
HardwareSerial hsBus2(2);       // used by Receiver for SBUS

SLog logger(&Serial, 115200, LOGLEVEL);   // setup up our serial Logger


// TaskClasses
CoopTask<void>* taskBlink = nullptr;  
CoopTask<void>* taskHover = nullptr;  
CoopTask<void>* taskSurface = nullptr;  
CoopTask<void>* taskOFlow = nullptr;  
CoopTask<void>* taskSteer = nullptr;  
CoopTask<void>* taskIdle = nullptr;  

CoopSemaphore taskSema(1,1);            // start-value 1, one concurrend task taskSema(<number>,<concurrendTasks>)
Blackbox bb(&logger, BLACKBOX_CS_PIN);
Receiver receiver(&logger, "RECV", &taskSema, &hsBus2, 16, 17, true, "AEHRD23T");
Mixer mixer(&logger, "MIXER", &taskSema);


char buffer[300];

/***************************************************************/
/*                                                             */
/* all task do have this kind of function. This is a callback. */
/* from CoopTask. Every task need such a callback function     */
/*                                                             */
/* currently only this tasks are available                     */
/*  taskBlink     Blink patterns                               */
/*  taskHover     for hovering                                 */
/*  taskSteering  for steering ()                              */
/*  taskSurface   for measuring distance to survace            */
/*  taskOpticalFlow for measuring an optical flow              */
/***************************************************************/


/* RUN this callBack function for endless loop this task */
void callbackTaskBlinkPattern() {
  unsigned long lastMillis = millis();
  BlinkPattern *obj = new BlinkPattern(&logger, "BLINK", TASK_BLINK, nullptr);
  uint8_t blink_pattern = 0;
  obj->begin();
  for(;;) {
    blink_pattern=PATTERN_IDLE;                 // if nothing special blink to have a heart-beat signal
    if (receiver.isPreventArming()) {           // special blink pattern if arming is not possible
      blink_pattern = PATTERN_PREVENTARMING;
    } else if (!receiver.isArmed()) {
        blink_pattern = PATTERN_DISARMED;       // blink pattern to show PODRacer is disarmed
    } else if (receiver.isArmed()) {
        blink_pattern = PATTERN_DISARMED;       // blink pattern to show PODRacer is disarmed
    }
    obj->update(blink_pattern, PREVENT_LOGGING_BLINK);
    yield();
  }
}

/* RUN this callBack function for endless loop this task */
void callbackTaskHover() {
  unsigned long lastMillis = millis();
  TaskHover *obj = new TaskHover(&logger, "HOVER", TASK_HOVER, &taskSema);
  #if defined(USE_TASK_HOVER)
  for(;;) {


  }
  #endif
}

/* RUN this callBack function for endless loop this task */
void callbackTaskSteering() {
  unsigned long lastMillis = millis();
  TaskSteering *obj = new TaskSteering(&logger, "STEER", TASK_STEERING, &taskSema);
  #if defined(USE_TASK_STEERING)
  for(;;) {


    
  }
  #endif
}

/* RUN this callBack function for endless loop this task */
void callbackTaskOpticalFlow() {
  unsigned long lastMillis = millis();
  TaskOpticalFlow *obj = new TaskOpticalFlow(&logger, "STEER", TASK_OPTICALFLOW, &taskSema);
  #if defined(USE_TASK_OPTICALFLOW)
  for(;;) {


    
  }
  #endif
}

/* RUN this callBack function for endless loop this task */
void callbackTaskSurface() {
  unsigned long lastMillis = millis();
  TaskSurface *obj = new TaskSurface(&logger, "SDIST", TASK_SURFACEDISTANCE, &taskSema);
  #if defined(USE_TASK_SURFACE)
  for(;;) {

    yield();
  }
#endif
}

/*-----------------------------------------------------------------------------------------*/
//
//  Arduino SETUP 
//
/*-----------------------------------------------------------------------------------------*/

void setup() {
  while (!Serial);
  delay(100);
  #if !defined(ESP32)
    Serial.println("!!!!!!!!!!!!!!!!!!!!!!!!!!!");
    Serial.println("not running on ESP32 device");
    Serial.println("!!!!!!!!!!!!!!!!!!!!!!!!!!!");
    assert(0);
  #endif

  sprintf(buffer, "- PODRacer-FWVersion: %s", bb.FWVersion());

  logger.info("------------------------------------------------", _tname);
  logger.info(buffer, _tname);
  logger.info("------------------------------------------------", _tname);
  logger.info("setup in progress....", _tname);
  bb.begin();

  taskBlink = new CoopTask<void>(F("BLINK"), callbackTaskBlinkPattern);

  logger.info("all tasks initialized, wakeup tasks...", _tname);
  if (taskBlink) taskBlink->wakeup();
  //if (taskIdle) taskBlink->wakeup();


  logger.info("all tasks running", _tname);

  logger.setVisualizerMode(LOG_OUTPUT_VISUALIZER_MODE);
  logger.setVisualizerMode(0);

}



/*-----------------------------------------------------------------------------------------*/
//
//  Arduino PROCESSING-LOOP 
//  
// This loop calls 
//  * read recevier SBUS-Data
//  * update tasks (this updates calculate their own task data)
//  * call mixer, the mixer calculate the next sbus-write sbus-data, based on task taskdata 
//  * write SBUS-Data based on mixer calculation
//  *    
/*-----------------------------------------------------------------------------------------*/
TaskData *tdr = nullptr;
TaskData *tdw = nullptr;
void loop() {
  //  in every loop we read the SBUS-Receiver
  tdr = tdw = nullptr;
  receiver.setMock(MOCK_RECEIVER_READ);
  receiver.read(tdr, PREVENT_LOGGING_RECEIVER);
  tdr = receiver.getTaskData();
  // run all tasks
  runCoopTasks(nullptr, nullptr, nullptr);
  // get latest receiver data structure
  // // update all task data due to latest receiver data structure
  // mixer.update(tdr);
  // // get the result from mixer
  // tdw = mixer.getTaskData();
  // // update SBUS-Receiver with a mixed signal
  // receiver.write(tdw);

  yield();
}