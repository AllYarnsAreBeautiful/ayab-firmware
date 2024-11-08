#include "hallsensor.h"
#include <stdlib.h>

#define MAX_DET_NEEDLES 3
#define NONE 0xffff

// TODO: Add interfce for a "digital" type (ESP32 & KH910 issue)
HallSensor::HallSensor(hardwareAbstraction::HalInterface *hal, uint8_t pin) {
  _hal = hal;
  _pin = pin;

  _hal->pinMode(_pin, INPUT);  // TODO: INPUT_PULLUP for KH910 right sensor

  _detectedPosition = 0;
  _detectedCarriage = CarriageType::NoCarriage;
  _detectedBeltPhase = false;

  _resetDetector();
  _readSensor();
}

void HallSensor::config(HallSensor::Config *config) {
  _config = config;
}

uint16_t HallSensor::getSensorValue() { return _sensorValue; }

bool HallSensor::isActive() {
  return (_minimum.value != NONE) || (_maximum.value != NONE);
}

uint8_t HallSensor::getSensorPosition() { return _config->position; }

int16_t HallSensor::getDetectedPosition() { return _detectedPosition; }

CarriageType HallSensor::getDetectedCarriage() { return _detectedCarriage; }

bool HallSensor::getDetectedBeltPhase() { return _detectedBeltPhase; }

bool HallSensor::isDetected(Encoder *encoder, Direction direction, bool beltPhase) {
  bool isDetected = false;
  int16_t encoder_position = encoder->getPosition();

  _readSensor();

  switch (_state) {
    case State::ST_HUNT:
      // Proceed as long as direction stay the same
      if (direction == _detectedDirection) {
        _needlesToGo--;

        // Record minimum position
        if (_sensorValue < _config->thresholdLow) {
          if ((_minimum.value == NONE) || (_sensorValue < _minimum.value)) {
            _minimum.value = _sensorValue;
            _minimum.position = encoder_position;
            _detectedBeltPhase = beltPhase;
            if (_maximum.value ==
                NONE) {  // Adjust trigger position to the extremum
              _needlesToGo = MAX_DET_NEEDLES;
              _detectedPosition = _minimum.position;
            }
          }
        }
        // Record maximum position
        if (_sensorValue > _config->thresholdHigh) {
          if ((_maximum.value == NONE) || (_sensorValue > _maximum.value)) {
            _maximum.value = _sensorValue;
            _maximum.position = encoder_position;
            _detectedBeltPhase = beltPhase;
            if (_minimum.value ==
                NONE) {  // Adjust trigger position to the extremum
              _needlesToGo = MAX_DET_NEEDLES;
              _detectedPosition = _maximum.position;
            }
          }
        }

        // Select carriage once max. needles passed
        if (_needlesToGo == 0) {
          // Detect only if carriage didn't change direction since first detection
          if (abs(encoder_position - _detectedPosition) == MAX_DET_NEEDLES) {
            isDetected = _detectCarriage();
          } 
          _resetDetector();
        }
      } else {
        // Detection not reliable, move out
        _state = State::ST_ESCAPE;
        _detectedPosition = encoder_position;
      }
    case State::ST_ESCAPE:
      // Hide until clearly out of the magnet window (at least 4 needles for G)
      if (abs(encoder_position - _detectedPosition) > 4) {
        _resetDetector();
      }
      break;

    default:  // ST_IDLE
      _state = State::ST_HUNT;
      _needlesToGo = MAX_DET_NEEDLES;
      _detectedBeltPhase = beltPhase;
      _detectedPosition = encoder_position;
      _detectedDirection = direction;
      if (_sensorValue < _config->thresholdLow) {
        _minimum = {.value = _sensorValue,
                    .position = _detectedPosition,
                    .isFirst = true};
      } else if (_sensorValue > _config->thresholdHigh) {
        _maximum = {.value = _sensorValue,
                    .position = _detectedPosition,
                    .isFirst = true};
      } else {
        _state = State::ST_IDLE;
      }
      break;
  }
  return isDetected;
}

void HallSensor::_resetDetector() {
  _state = State::ST_IDLE;
  _minimum = {.value = NONE, .position = 0, .isFirst = false};
  _maximum = {.value = NONE, .position = 0, .isFirst = false};
}

bool HallSensor::_detectCarriage() {
  bool isDetected = true;

  if (_minimum.value == NONE) {
    // K Carriage (only a maximum/North pole)
    _detectedCarriage = CarriageType::Knit;
    _detectedPosition = _maximum.position;
    if ((_config->flags & HALLSENSOR_K270_K) != 0) {
    // 'K' carriage on KH270 (left/K magnet enabled)
      _detectedCarriage = CarriageType::Knit270;
    } else if ((_config->flags & HALLSENSOR_L_HIGH) != 0) {
      // Digital signal for L is active high (KH910 RHS)
      _detectedCarriage = CarriageType::Lace;
    }
  } else if (_maximum.value == NONE) {
    // L Carriage (only a minimum/South pole)
    _detectedCarriage = CarriageType::Lace;
    _detectedPosition = _minimum.position;
    if ((_config->flags & HALLSENSOR_K_LOW) != 0) {
      // Digital signal for K is active Low (KH910 RHS)
      _detectedCarriage = CarriageType::Knit;
    }
  } else if (_minimum.isFirst) {
    // G Carriage (minimum/South followed by maximum/North poles)
    _detectedCarriage = CarriageType::Garter;
    _detectedPosition = _minimum.position;
  } else {
    isDetected = false;
  }

  return isDetected;
}

//TODO: Move analog or single/dual digital read here
void HallSensor::_readSensor() { _sensorValue = _hal->analogRead(_pin); }