#include "solenoids.h"

Solenoids::Solenoids(hardwareAbstraction::HalInterface *hal,
                     const uint8_t i2cAddress[2]) {
  for (int i = 0; i < 2; i++) {
    _mcp23008[i] = new Mcp23008(hal, i2cAddress[i]);
    _mcp23008[i]->write(MCP23008_IODIR, 0);  // Configure as output
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
    _mcp23008[i]->write(MCP23008_OLAT, (uint8_t)(values & 0xff));
    values >>= 8;
  }
}