#ifndef HALLSENSOR_H
#define HALLSENSOR_H

#include "api.h"
#include "belt.h"
#include "encoder.h"
#include "hal.h"

class HallSensor {
 public:
  class Config {
   public:
    uint8_t position;
    uint16_t thresholdLow;
    uint16_t thresholdHigh;
  };

  HallSensor(hardwareAbstraction::HalInterface *hal, uint8_t pin);
  ~HallSensor() = default;

  // Config sensor's position and thresholds
  void config(Config *config);

  // Return last sensor value
  uint16_t getSensorValue();
  // Return true when a threshold has been crossed
  bool isActive();
  // Return sensor (needle) position
  uint8_t getSensorPosition();

  // Return encoder position when carriage was detected
  uint8_t getDetectedPosition();
  // Return detected carriage type
  CarriageType getDetectedCarriage();
  // Return detected belt phase
  bool getDetectedBeltPhase();

  // Run internal detection state machine and return its status
  bool isDetected(Encoder *encoder, Belt *belt);

 private:
  class Extremum {
   public:
    uint16_t value;
    uint8_t position;
    bool isFirst;
  };

  // Measure sensor voltage
  void _readSensor();
  // Reset detection state machine
  void _resetDetector();
  // Check is a valid carriage has been detected and update internal state
  bool _detectCarriage();

  hardwareAbstraction::HalInterface *_hal;

  uint8_t _pin;

  Config *_config;

  uint16_t _sensorValue;
  uint16_t _thresholdLow;
  uint16_t _thresholdHigh;
  bool _isKasG;

  uint8_t _detectedPosition;
  CarriageType _detectedCarriage;
  bool _detectedBeltPhase;

  uint8_t _state;
  Extremum _minimum;
  Extremum _maximum;
  int _needlesToGo;
};

#endif