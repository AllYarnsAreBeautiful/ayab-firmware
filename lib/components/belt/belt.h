#ifndef BELT_H
#define BELT_H

#include "api.h"
#include "hal.h"

class Belt {
 public:
  Belt(hardwareAbstraction::HalInterface *hal, uint8_t pin);
  ~Belt() = default;

  void schedule();

  // Get current belt phase signal
  bool getShift();

 private:
  hardwareAbstraction::HalInterface *_hal;
  uint8_t _pin;
  bool _shift;
};

#endif