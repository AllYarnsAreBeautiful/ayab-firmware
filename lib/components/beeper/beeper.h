#ifndef BEEPER_H
#define BEEPER_H

#include "hal.h"

class Beeper {
 public:
  Beeper(hardwareAbstraction::HalInterface *hal, uint8_t pin);
  ~Beeper() = default;

  void beep(uint8_t number);
  void schedule();

 private:
  enum class State { Idle, Off, On, Wait };

  hardwareAbstraction::HalInterface *_hal;
  uint8_t _pin;
  uint8_t _number;
  State _state, _nextState;
  unsigned long _nextTime;

  void _write(uint8_t value);
};

#endif