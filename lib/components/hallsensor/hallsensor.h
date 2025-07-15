#ifndef HALLSENSOR_H
#define HALLSENSOR_H

#include "api.h"
#include "encoder.h"
#include "hal.h"

// No pin indication
#define PIN_NONE 0xff

// Hall sensor flags
#define HALLSENSOR_DIGITAL (1<<0)
#define HALLSENSOR_K270_K  (1<<1)

class HallSensor {
 public:
  class Config {
   public:
    int16_t position;
    uint16_t thresholdLow;
    uint16_t thresholdHigh;
    uint8_t flags;
  };

  HallSensor(hardwareAbstraction::HalInterface *hal, uint8_t sensorPin1, uint8_t sensorPin2 = PIN_NONE, uint8_t detectPin = PIN_NONE);
  ~HallSensor() = default;

  // Config sensor's position and thresholds
  void config(Config *config);
  // Return last sensor value
  uint16_t getSensorValue();
  // Return true when a threshold has been crossed
  bool isActive();
  // Return sensor (needle) position
  int16_t getSensorPosition();
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

  uint8_t _sensorPin1, _sensorPin2;

  Config *_config;

  uint16_t _sensorValue;
  uint16_t _thresholdLow;
  uint16_t _thresholdHigh;
  bool _isPin2Wired; // KH910 HW fix for RHS sensor is present

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