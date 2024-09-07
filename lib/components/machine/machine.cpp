#include "machine.h"

HallSensor::Config leftSensorConfig, rightSensorConfig;

Machine::Machine() { reset(); }

void Machine::reset() { _type = MachineType::NoMachine; }

void Machine::setType(MachineType type) { _type = type; }

bool Machine::isDefined() { return _type != MachineType::NoMachine; }

uint8_t Machine::getNumberofNeedles() {
  switch (_type) {
    case MachineType::Kh270:
      return 112;
    default:
      return 200;
  }
}

HallSensor::Config *Machine::getSensorConfig(Side side) {
  HallSensor::Config *sensorConfig;

  if (side == Side::Left) {
    sensorConfig = &leftSensorConfig;
    sensorConfig->position = 0;
  } else {
    sensorConfig = &rightSensorConfig;
    sensorConfig->position = (uint8_t)(getNumberofNeedles() - 1);
  }

  // Default threshold settings
  sensorConfig->thresholdLow = 200;
  sensorConfig->thresholdHigh = 600;
  if ((_type == MachineType::Kh910) && (side == Side::Right)) {
    // KH910 unmodified; incorrect shield implementation, only North pole as digital low
    sensorConfig->thresholdLow = 100;
    sensorConfig->thresholdHigh = 1024;
    sensorConfig->flags |= HALLSENSOR_K_LOW;
  }

  return sensorConfig;
}

uint8_t Machine::solenoidToSet(uint8_t needleToSet) {
  switch (_type) {
    case MachineType::Kh270:
      return needleToSet % 12;
    default:
      return needleToSet % 16;
  }
}

void Machine::solenoidShift(uint8_t &solenoid) {
  switch (_type) {
    case MachineType::Kh270:
      solenoid = (solenoid + 6) % 12;
      break;
    default:
      solenoid = (solenoid + 8) % 16;
  }
}