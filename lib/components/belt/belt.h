#ifndef BELT_H
#define BELT_H

#include "api.h"
#include "hal.h"

class Belt {
 public:
  Belt(hardwareAbstraction::HalInterface *hal, uint8_t pin);
  ~Belt() = default;

  // Set belt shift (phase)
  void setshift(Direction direction, CarriageType carriageType);
  // Get current belt shift (phase)
  BeltShift getShift();

 private:
  hardwareAbstraction::HalInterface *_hal;
  uint8_t _pin;
  bool _shift;
};

#endif