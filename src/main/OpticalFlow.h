#ifndef _OPTICAL_FLOW_H_
#define _OPTICAL_FLOW_H_

#include <Arduino.h>
#include "SimpleLog.cpp"
//#include "TaskData.h"
#include "Task.h"
#include <Bitcraze_PMW3901.h>
#include <PID_v1.h>
#include "constants.h"
#include "Receiver.h"


/**
    Use a PMW3901 optical flow sensor

    This sensor is used to check if PODRacer is on a movement over ground in +/-X and +/-Y direction

            -Y
            |
      +X-Y  |  -X-Y
    +X -----|-------
      +X+Y  |  -X+Y 
            |
            +Y
        <SOUTH>
    wiring-connector

    It's normal that the PODRace slip forward/backward/sideways(left/right) due on weather conditions (wind), bad calibrated hardware, flight controler setup bad
    a.s.o

    During hovering, the PODRacer should not slip in any direction. With the PMW3901 we recognise any directions and calculate new receiver channel commands.

    Example:
      Forward-Slip:
      the PODRace slip during hovering forward. That means rear Motors are faster than front motors (or Thrust Prop is to fast).
      To to into back direction the FlightControler should increase spead of front motors this will be done if pitch command is set to backward (GIMBAL move down)

      Backward-Slip:
      pitch command is set to forward (gimbal move up)

      Left Slip:
      correct with roll - ROLL Gimbal move right

      Reight Slip      
      correct with roll - ROLL Gimbal move left

      To avoid an overshoting a PID-Controller is used for adjust slip values

      This adjusted slip value (for X/Y) is a used to calculate new channel values (Values between 1000-2000).
      No - slippering - channel value for roll, pitch, yaw should be 1500



  **/

class OpticalFlow : public TaskAbstract {
public:
  OpticalFlow(uint8_t taskID, SLog *log, uint8_t cs_pin, Blackbox *bb = nullptr, HardwareSerial *visBus=nullptr);
  bool begin(void) {
    ;
  };
  bool begin(Receiver *recveiver);
  void update(void);

private:
  Receiver *_recv;
  //TDATA data;
  Bitcraze_PMW3901 *flow;
  uint8_t _cs;

  // rawX/Y store the raw sensor values for X/Y
  // we add this values to the slipping X/Y values
  int16_t rawX, rawY;
  int16_t rawAdjX, rawAdjY;
  long rawXnormalized, rawYnormalized;

  // the slipping value for X and Y direction
  // roll axis - slip to left/right
  // pitch axis - slip to forward/backward
  double slip2RollAxis, slip2PitchAxis;

  // the adjusted slipping value for X/Y
  double slipAdjX, slipAdjY;

  // the PID controllyer should try to get this setpoint
  double setPointSlipX, setPointSlipY;
  double biasRoll = 1.0;
  double biasPitch = 1.0;

  //----- PID Controller for OpticalFlow sensor
  double kpOpticalFlow = 0.5;
  double kiOpticalFlow = 0.05;  //0.9
  double kdOpticalFlow = 0;     //3.6

  PID *pidX, *pidY;
  uint8_t flowCounter;

  // north = wiring pins are south, north=flight direction
  //
  // [0]    direction                 : 0=North, 1=WEST, 2=EAST, 3=SOUTH
  // [1..4] rawXY multiplier +1/-1    : [1] Multiplier for X [2] Multiplier for Y
  // Example : NORTH direction        : [0][+1][-1]
  // Example : WEST direction         : [1][-1][+1]
  // Example : EAST direction         : [2][+1][-1]
  // Example : SOUTH direction        : [3][-1][+1]

  int8_t direction[3] = { 0, +1, -1 };
  uint8_t rpy[3];


  uint8_t cnt;
};

#endif
