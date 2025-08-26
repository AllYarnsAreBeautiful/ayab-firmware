#include <Wire.h>
#include "i2c.h"

namespace hardwareAbstraction {

I2c::I2c() { ::Wire.begin(); }

bool I2c::detect(uint8_t device) {
  ::Wire.beginTransmission(device);
  return ::Wire.endTransmission() == 0;
}

uint8_t I2c::read(uint8_t device, uint8_t address) {
  uint8_t value;
  ::Wire.beginTransmission(device);
  ::Wire.write(address);  
  ::Wire.endTransmission(false);  
  ::Wire.requestFrom(address, 1);
  if (Wire.available()) {
    value = ::Wire.read();
  } // FIXME: should signal an error to the desktop app otherwise ?
  ::Wire.endTransmission();
  return value;
}

void I2c::write(uint8_t device, uint8_t value) {
  ::Wire.beginTransmission(device);
  ::Wire.write(value);
  ::Wire.endTransmission();
}

void I2c::write(uint8_t device, uint8_t address, uint8_t value) {
  ::Wire.beginTransmission(device);
  ::Wire.write(address);
  ::Wire.write(value);
  ::Wire.endTransmission();
}
}  // namespace hardwareAbstraction