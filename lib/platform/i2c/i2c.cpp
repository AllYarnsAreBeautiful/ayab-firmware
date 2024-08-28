#include "i2c.h"

#if defined(__AVR_ATmega168__) || defined(__AVR_ATmega328P__)
#define I2C_HARDWARE 1
#define SDA_PORT PORTC
#define SDA_PIN 4  // = A4
#define SCL_PORT PORTC
#define SCL_PIN 5  // = A5
#elif defined(__AVR_ATmega2560__) || defined(__AVR_ATmega1280__)
#define SDA_PORT PORTF
#define SDA_PIN 4  // = A4
#define SCL_PORT PORTF
#define SCL_PIN 5  // = A5
#else
#warning untested board - please check your I2C ports
#endif

#include <SoftI2CMaster.h>
// #include <Wire.h>

namespace hardwareAbstraction {

I2c::I2c() { ::i2c_init(); }

void I2c::write(uint8_t device, uint8_t address, uint8_t value) {
  ::i2c_start((device << 1) | I2C_WRITE);
  ::i2c_write(address);
  ::i2c_write(value);
  ::i2c_stop();
}
}  // namespace hardwareAbstraction