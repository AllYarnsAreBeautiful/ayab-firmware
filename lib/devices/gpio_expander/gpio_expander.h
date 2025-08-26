#ifndef GPIOEXPANDER_H
#define GPIOEXPANDER_H

#include "hal.h"

class GpioExpander {
 public:
  GpioExpander(hardwareAbstraction::HalInterface *hal, const uint8_t i2cAddress);
  ~GpioExpander() = default;

  // Virtual method for derived classes to implement
  virtual void write(uint8_t address, uint8_t value) = 0;

  hardwareAbstraction::HalInterface *_hal;
  uint8_t _i2cAddress;
  uint8_t _olat_cache;
  bool _olat_cache_invalid;
};

#endif