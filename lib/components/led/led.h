#ifndef LED_H
#define LED_H

#include "hal.h"

class Led {
 public:
  Led(hardwareAbstraction::HalInterface *hal, uint8_t pin);
  Led(hardwareAbstraction::HalInterface *hal, uint8_t pin, uint8_t offValue,
      uint8_t onValue);
  ~Led() = default;

  // Switch led on
  void on();
  // Switch led off
  void off();
  // Toggle led state
  void toggle();
  // Define a periodic off/on led pattern
  void blink(unsigned long timeOff, unsigned long timeOn);
  // Flash once for a given duration
  void flash(unsigned long timeOn);
  // Update internal state machine
  void schedule();

 private:
  enum class State { Off, On };
  enum class Mode { Solid, Blink, Flash };

  hardwareAbstraction::HalInterface *_hal;
  uint8_t _pin;
  State _state;
  Mode _mode, _savedMode;
  uint8_t _offValue, _onValue;

  unsigned long _timeOff, _timeOn, _flashOn;
  unsigned long _lasttime;

  void _write();
};

#endif
