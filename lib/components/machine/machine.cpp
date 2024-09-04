#include "machine.h"

HallSensor::Config leftSensor, rightSensor;

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

HallSensor::Config *Machine::getSensorConfig(Direction direction) {
  HallSensor::Config *sensor;

  if (direction == Direction::Left) {
    sensor = &leftSensor;
    sensor->position = 0;
  } else {
    sensor = &rightSensor;
    sensor->position = (uint8_t)(getNumberofNeedles() - 1);
  }

  // Default threshold settings
  sensor->thresholdLow = 200;
  sensor->thresholdHigh = 600;
  if ((_type == MachineType::Kh910) && (direction == Direction::Right)) {
    // Incorrect shield implementation, only North pole can be detected as South
    sensor->thresholdLow = 100;
    sensor->thresholdHigh = 1024;
  }

  return sensor;
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