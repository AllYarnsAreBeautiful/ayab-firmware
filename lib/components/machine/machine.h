#ifndef MACHINE_H
#define MACHINE_H

#include <stdint.h>

#include "api.h"
#include "hallsensor.h"

class Machine {
 public:
  Machine();
  ~Machine() = default;

  // Reset machine state
  void reset();
  // Set machine type
  void setType(MachineType type);
  // get machine type
  bool isDefined();
  // Return number of needles for the machine
  uint8_t getNumberofNeedles();
  // Return characteristics of the left or right sensor
  HallSensor::Config* getSensorConfig(MachineSide side);
  // Compute solenoid to set for a given needle position
  uint8_t solenoidToSet(uint8_t needleToSet);
  // Shift solenoid bank
  void solenoidShift(uint8_t& solenoid);
  // Map solenoids to driver outputs
  void solenoidMap(uint8_t &solenoid);

 private:
  MachineType _type;
};

#endif