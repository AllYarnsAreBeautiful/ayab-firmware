#include "solenoids.h"

Solenoids::Solenoids(hardwareAbstraction::HalInterface *hal,
                      GpioExpander* const gpio_expander[2]) {
  _gpio_expander[0] = gpio_expander[0];
  _gpio_expander[1] = gpio_expander[1];
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
    if (_gpio_expander[i]) {
      _gpio_expander[i]->update((uint8_t)(values & 0xff));
    }
    values >>= 8;
  }
}