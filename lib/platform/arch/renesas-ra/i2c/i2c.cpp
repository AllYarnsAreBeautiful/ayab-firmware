#include <Wire.h>
#include "i2c.h"

namespace hardwareAbstraction {

I2c::I2c() { ::Wire.begin(); }

void I2c::write(uint8_t device, uint8_t address, uint8_t value) {
  ::Wire.beginTransmission(device);
  ::Wire.write(address);
  ::Wire.write(value);
  ::Wire.endTransmission();
}
}  // namespace hardwareAbstraction