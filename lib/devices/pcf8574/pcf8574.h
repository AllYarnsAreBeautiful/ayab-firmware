#ifndef PCF8574_H
#define PCF8574_H

#include "hal.h"
#include "../gpio_expander/gpio_expander.h"

class Pcf8574: public GpioExpander {
 public:
  Pcf8574(hardwareAbstraction::HalInterface *hal, uint8_t i2cAddress): GpioExpander(hal, i2cAddress) {};
  ~Pcf8574() = default;

  // Update output latch register
  void update(uint8_t value) override;
};

#endif