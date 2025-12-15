#ifndef SOLENOIDS_H
#define SOLENOIDS_H

#include "hal.h"
#include "gpio_expander.h"

#define SOLENOID_RESET_STATE 0xffff

class Solenoids {
 public:
  Solenoids(hardwareAbstraction::HalInterface *hal,
            GpioExpander* const gpio_expander[2]);
  ~Solenoids() = default;

  // Reset all solenoids
  void reset();
  // Reset on solenoid
  void reset(uint8_t solenoid);
  // Set all solenoids
  void set(uint16_t state);
  // Set one solenoid
  void set(uint8_t solenoid, bool state);

 private:
  GpioExpander* _gpio_expander[2];
  uint16_t _states;

  // Update devices with current object state
  void _updateDevices();
};

#endif