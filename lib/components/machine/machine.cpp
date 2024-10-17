#include "machine.h"

HallSensor::Config leftSensorConfig, rightSensorConfig;

Machine::Machine() { reset(); }

void Machine::reset() { _type = MachineType::NoMachine; }

void Machine::setType(MachineType type) { _type = type; }

MachineType Machine::getType() { return _type; }

bool Machine::isDefined() { return _type != MachineType::NoMachine; }

uint8_t Machine::getNumberofNeedles() {
  switch (_type) {
    case MachineType::Kh270:
      return 112;
    default:
      return 200;
  }
}

HallSensor::Config *Machine::getSensorConfig(MachineSide side) {
  HallSensor::Config *sensorConfig;

  if (side == MachineSide::Left) {
    sensorConfig = &leftSensorConfig;
    sensorConfig->position = 0;
  } else {
    sensorConfig = &rightSensorConfig;
    sensorConfig->position = (uint8_t)(getNumberofNeedles() - 1);
  }

  // Default threshold settings
  sensorConfig->thresholdLow = 200;
  sensorConfig->thresholdHigh = 600;
  if ((_type == MachineType::Kh910) && (side == MachineSide::Right)) {
    // KH910 unmodified; incorrect shield implementation, only North pole as digital low
    sensorConfig->thresholdLow = 100;
    sensorConfig->thresholdHigh = 1024;
    sensorConfig->flags |= HALLSENSOR_K_LOW;
  } else if (_type == MachineType::Kh270) {
    sensorConfig->flags |= HALLSENSOR_K270_K;
  }

  return sensorConfig;
}

uint8_t Machine::solenoidToSet(uint8_t needleToSet) {
  switch (_type) {
    case MachineType::Kh270:
      return (needleToSet + 4) % 12;
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

void Machine::solenoidMap(uint8_t &solenoid) {
  if (_type == MachineType::Kh270) {
    // 16 solenoids mapped to output 3-14
    solenoid += 3;
  }
}
