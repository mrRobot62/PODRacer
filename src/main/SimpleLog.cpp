
#ifndef _SIMPLELOG_
#define _SIMPLELOG_
#include <HardwareSerial.h>
#include "constants.h"

//namespace podr {
  class SLog {
    public:
      SLog(HardwareSerial *bus, int baud=115200, uint8_t level=3) {
        _bus = bus;
        _bus->begin(baud);
        _level = level;
      }
      void setLoglevel(uint8_t level=3) {
        _level = level;
      }
      void error(const long v, const char *tname="?", bool cr=true) {
        sprintf(buffer, "%i", v);
        error(buffer, tname, cr);
      }
      void error(const char *text, const char *tname="?", bool cr=true) {
        if (_level >= 1) {
          _print(text, "ERROR", cr, false, tname);
        }
      }
      void warn(const long v, const char *tname="?", bool cr=true) {
        sprintf(buffer, "%i", v);
        warn(buffer, tname, cr);
      }
      void warn(const char *text, const char *tname="?", bool cr=true) {
        if (_level >= 2) {
          _print(text, "WARN", cr,false,tname);
        }
      }

      void simulate(const char *text, const char *tname="?") {
        _print(text, "SIMUALTE", true, true,tname);
      }

      void info(const long v, const char *tname="?", bool cr=true) {
        sprintf(buffer, "%i", v);
        info(buffer, tname, cr);
      }

      void info(const double v, const char *tname="?", bool cr=true) {
        sprintf(buffer, "%f", v);
        info(buffer, tname, cr);
      }
      void info(const char *text, const char *tname="?", bool cr=true) {
        if (_level >= 3) {
          _print(text, "INFO", cr,false,tname);
        }
      }

      void print(const long v, bool cr=true) {
        sprintf(buffer, "%i", v);
        print(buffer, cr);
      }

      void print(const char *text, bool cr=false) {
        if (cr) {
          _bus->println(text);
        }
        else {
          _bus->print(text);
        }
      }

      void printBinary(const char *text, const char* tname, const uint8_t v, bool cr=true) {
        info(text, tname, false);
        sprintf(buffer, BYTE_TO_BINARY_PATTERN, BYTE_TO_BINARY(v));
        if (cr) {
          _bus->println(buffer);
        }
        else {
          _bus->print(buffer);
        }
      }

      /** convert v into a bit buffer array **/
      char* getBinary(const uint8_t v, uint8_t pattern=1) {
        switch(pattern) {
          case 0:
            sprintf(buffer, BYTE_TO_BINARY_PATTERN, BYTE_TO_BINARY(v));
            break;
          case 1:
            sprintf(buffer, BYTE_TO_BINARY_PATTERN2, BYTE_TO_BINARY(v));
            break;
          default:
            sprintf(buffer, BYTE_TO_BINARY_PATTERN, BYTE_TO_BINARY(v));
        }
        return buffer;
      }

      void debug(const long v, const char *tname="?", bool cr=true) {
        sprintf(buffer, "%d", v);
        debug(buffer,tname, cr);
      }
      void debug(const char *text, const char *tname="?", bool cr=true) {
        if (_level >= 4) {
          _print(text, "DEBUG", cr,false,tname);
        } 
      }

    private:
      HardwareSerial *_bus;
      uint8_t _level;
      char buffer [300];
    private:
      void _print(const char *text, const char *LEVEL, bool cr=true, bool simulate=false, const char *tname="?") {
        #if not defined (LOG_VISUALIZER)

        if (simulate) {
          _bus->println(text);
        }
        else {
          sprintf(buffer,"%10d|%5s|%5s|%s", millis(), LEVEL, tname, text);
          if (cr) {
            _bus->println(buffer);
          }
          else {
            _bus->print(buffer);
        }
        }
        _bus->flush();
        #endif 
      }

  };

//};


#endif