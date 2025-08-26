#include "gpio_expander.h"

GpioExpander::GpioExpander(hardwareAbstraction::HalInterface *hal, const uint8_t i2cAddress) {
  _hal = hal;
  _i2cAddress = i2cAddress;
  _olat_cache_invalid = true;
}