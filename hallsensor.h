#include "Arduino.h"
#include "./settings.h" 

#define MAX_DET_NEEDLES 3
#define NONE 0xffff

#define ST_IDLE 0
#define ST_HUNT 1

struct Extremum {
  uint16_t value;
  uint16_t position;
  bool     isFirst;
};

class HallSensor {
  public:
    HallSensor(int pin);

    Carriage_t  carriage;
    byte        position;
    Beltshift_t beltShift;

    void setThresholds(uint16_t low, uint16_t high);
    bool isDetected(byte &position, Direction_t &direction);
    bool isActive();
    uint16_t getValue();

  private:
    int _pin;
    uint16_t _sensorValue;
    uint16_t _thresholdLow;
    uint16_t _thresholdHigh;

    byte        _state;
    Extremum    _minimum;
    Extremum    _maximum;
    Direction_t _direction;
    int         _needlesToGo;

    void _init();
    bool _detectCarriage();
};
