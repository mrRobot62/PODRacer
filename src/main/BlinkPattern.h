#ifndef _BlinkPattern_H_
#define _BlinkPattern_H_

#include <Arduino.h>
#include "Task.h"
#include "constants.h"

#define MAX_PATTERN 10

#define PATTERN_PREVENTARMING 9
#define PATTERN_DISARMED 8
#define PATTERN_EMERGENCY 7

class BlinkPattern : public TaskAbstract {
  public:
    BlinkPattern(uint8_t taskID, SLog *log, Blackbox *bb=nullptr);

    /** initialize **/
    bool begin(void) {;};
    bool begin(uint8_t *blink_pattern);
    /** update site loop **/
    void update(void);
    void update(uint8_t pattern) ;

    inline void setPattern(uint8_t pattern) {
      cPattern = constrain(pattern, 0, MAX_PATTERN-1 );
    }

  private:
    /**
      0b00000000 = Bit pattern 1=ON, 0=OFF (from right to left)
      ON-TIME in ms
      OFF-time in ms
      PIN

      {0b00001010, 500, 100, 2}
      0b00001010    OFF   ON   OFF  ON  OFF OFF OFF OFF
                    100   500  100  500 100 100 100 100
      LED on pin 2 (build-in led)
      
      Note: if ON an internal 100ms delay is done
    **/
    uint8_t cPattern, defaultPattern;
    uint8_t idx[8] = {1,2,4,8,16,32,64,128};
    uint8_t pins[2] = {2,15};
    uint16_t pattern[MAX_PATTERN][4] = {
      {0b00000001, 500, 200, 15}, // 0 TASK_HB (blink 1x in 2secs (7xoff with 200ms =1400ms + 1xON with 500ms = 1900ms +100ms(internal) = 2000ms blink))
      {0b00000000, 100, 200, 15}, // 1 
      {0b00000000, 500, 200, 15}, // 2 
      {0b00000000, 250, 250, 15}, // 3 
      {0b00000000, 250, 250, 15}, // 4 
      {0b00000000, 100,  50, 15}, // 5 
      {0b00000000, 250, 250, 15}, // 6 
      {0b11111111,  75,  50, 15}, // 7 Emergency - very fast blinking
      {0b00110011, 250, 100, 15}, // 8 disarmed
      {0b11111111, 150, 100, 15}  // 9 preventArming !
    };
 
};

#endif