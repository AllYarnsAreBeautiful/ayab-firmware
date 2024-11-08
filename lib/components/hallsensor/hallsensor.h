#ifndef HALLSENSOR_H
#define HALLSENSOR_H

#include "api.h"
#include "encoder.h"
#include "hal.h"

#define HALLSENSOR_K_LOW  (1<<0)
#define HALLSENSOR_L_HIGH (1<<1)
#define HALLSENSOR_K270_K (1<<2)

class HallSensor {
 public:
  class Config {
   public:
    uint8_t position;
    uint16_t thresholdLow;
    uint16_t thresholdHigh;
    uint8_t flags;
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
  int16_t getDetectedPosition();
  // Return detected carriage type
  CarriageType getDetectedCarriage();
  // Return detected belt phase
  bool getDetectedBeltPhase();

  // Run internal detection state machine and return its status
  bool isDetected(Encoder *encoder, Direction direction, bool beltPhase);

 private:
  enum class State { ST_IDLE, ST_HUNT, ST_ESCAPE };

  class Extremum {
   public:
    uint16_t value;
    int16_t position;
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

  int16_t _detectedPosition;
  CarriageType _detectedCarriage;
  bool _detectedBeltPhase;
  Direction _detectedDirection;

  State _state;
  Extremum _minimum;
  Extremum _maximum;
  uint8_t _needlesToGo;
};

#endif