#include "solenoids.h"

Solenoids::Solenoids(hardwareAbstraction::HalInterface *hal,
                     const uint8_t i2cAddress[][2]) {

  // Detect presence of both GPIO expanders
  // FIXME: First one is selected when none are detected -> should raise an error towards desktop app instead
  int i2C_address_set = 0;
  for (int id = 0; (i2cAddress[id][0] != 0) || (i2cAddress[id][1] != 0); id++) {
    if (hal->i2c->detect(i2cAddress[id][0]) && hal->i2c->detect(i2cAddress[id][1])) {
      i2C_address_set = id;
      break;
    };
  }

  for (int i = 0; i < 2; i++) {
    // Detect GPIO expander type
    // MCP23008 IOCON.0 always reads as 0 while PCF8574 will latch the last written value
    hal->i2c->write(i2cAddress[i2C_address_set][i], MCP23008_IOCON, 0x01);
    if ((hal->i2c->read(i2cAddress[i2C_address_set][i], MCP23008_IOCON) & 0x01) == 0x00) {
      _gpio_expander[i] = (GpioExpander *) new Mcp23008(hal, i2cAddress[i2C_address_set][i]);  
      _gpio_expander[i]->write(MCP23008_IODIR, 0);  // Configure as output
    } else {
      _gpio_expander[i] = (GpioExpander *) new Pcf8574(hal, i2cAddress[i2C_address_set][i]);
    }
  }
  reset();
}

void Solenoids::reset() {
  set(SOLENOID_RESET_STATE);
}

void Solenoids::reset(uint8_t solenoid) {
  set(solenoid, (bool)SOLENOID_RESET_STATE);
}

void Solenoids::set(uint16_t states) {
  _states = states;
  _updateDevices();
}

void Solenoids::set(uint8_t solenoid, bool state) {
  if (state) {
    _states |= (1 << solenoid);
  } else {
    _states &= ~(1 << solenoid);
  }
  _updateDevices();
}

void Solenoids::_updateDevices() {
  uint16_t values = _states;
  for (int i = 0; i < 2; i++) {
    _gpio_expander[i]->write(MCP23008_OLAT, (uint8_t)(values & 0xff));
    values >>= 8;
  }
}