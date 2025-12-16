#ifndef GPIOEXPANDER_H
#define GPIOEXPANDER_H

#include "hal.h"

class GpioExpander {
 public:
  GpioExpander(hardwareAbstraction::HalInterface *hal, const uint8_t i2cAddress);
  virtual ~GpioExpander() = default;

  // Virtual method for derived classes to implement
  virtual void update(uint8_t value) = 0;

  hardwareAbstraction::HalInterface *_hal;
  uint8_t _i2cAddress;
  uint8_t _output_latch_cache;
  bool _cache_invalid;
};

class DummyExpander final : public GpioExpander {
 public:

  DummyExpander(hardwareAbstraction::HalInterface *hal, uint8_t i2cAddress)
      : GpioExpander(hal, i2cAddress) {};
  ~DummyExpander() = default;

  // Update output latch register
  void update(uint8_t value) override;
};

#endif