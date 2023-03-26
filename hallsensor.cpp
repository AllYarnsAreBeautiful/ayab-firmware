#include "Arduino.h"

#include "./hallsensor.h"

HallSensor::HallSensor(int pin) {
  _pin = pin;
  _sensorValue = analogRead(_pin);
  
  carriage = NoCarriage;
  _init();
}

void HallSensor::setThresholds(uint16_t low, uint16_t high) {
  _thresholdLow = low;
  _thresholdHigh = high;
}

bool HallSensor::isDetected(byte &position, Direction_t &direction) {

  bool isDetected = false;

  _sensorValue = getValue();

  switch (_state) {
    case ST_IDLE:
      _state = ST_HUNT;
      _direction = direction;
      _needlesToGo = MAX_DET_NEEDLES;

      if (_sensorValue < _thresholdLow) {
        _minimum = {.value = _sensorValue, .position = position, .isFirst = true};
      } else if (_sensorValue > _thresholdHigh) {
        _maximum = {.value = _sensorValue, .position = position, .isFirst = true};
      } else {
        _state = ST_IDLE;
      }
      break;

    default: // ST_HUNT
      // Bail out if direction changed
      if (_direction != direction) {
         _init();
         break;
      }

      // Record minimum position
      if (_sensorValue < _thresholdLow) {
        if ((_minimum.value == NONE) || (_sensorValue < _minimum.value)) {
          _minimum.value    = _sensorValue;
          _minimum.position = position;
          if (_maximum.value == NONE) { // Adjust trigger position to the extremum
            _needlesToGo = MAX_DET_NEEDLES;
          }
        }
      }
      // Record maximum position
      if (_sensorValue > _thresholdHigh) {
        if ((_maximum.value == NONE) || (_sensorValue > _maximum.value)) {
          _maximum.value    = _sensorValue;
          _maximum.position = position;
          if (_minimum.value == NONE) { // Adjust trigger position to the extremum
            _needlesToGo = MAX_DET_NEEDLES;
          }
        }
      }

     // Select carriage once max. needles passed
      _needlesToGo--;
      if (_needlesToGo == 0) {
        isDetected = _detectCarriage();
        _init();
      }
  }
  return isDetected;
}

bool HallSensor::isActive() {
  return _state != ST_IDLE;
}

uint16_t HallSensor::getValue() {
  // Return stored value rather than thru another analogRead (time-consuming/blocking) ?
  // => only updated when upon isActive() calls
  //return _sensorValue;
  return  analogRead(_pin);
}

void HallSensor::_init() {
  _state = ST_IDLE;
  _minimum = {.value = NONE, .position = NONE, .isFirst = false};
  _maximum = {.value = NONE, .position = NONE, .isFirst = false};
}

bool HallSensor::_detectCarriage() {
  bool isDetected = true;

  // K Carriage (only a maximum/North pole)
  if (_minimum.value == NONE) {
    carriage = K;
    position = (byte) (_maximum.position & 0xff);
  // L Carriage (only a minimum/South pole)
  } else if (_maximum.value == NONE) {
    carriage = L;
    position = (byte) (_minimum.position & 0xff);
  // G Carriage (maximum/North followed by minimum/South poles)
  } else if (_maximum.isFirst) {
    carriage = G;
    position = (byte) (((_minimum.position + _maximum.position) >> 1) & 0xff);
  } else {
    isDetected = false;
  }

  return isDetected;
}
