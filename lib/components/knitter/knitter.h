#ifndef KNITTER_H
#define KNITTER_H

#include "beeper.h"
#include "carriage.h"
#include "encoder.h"
#include "hal.h"
#include "hallsensor.h"
#include "led.h"
#include "line.h"
#include "machine.h"
#include "solenoids.h"

#define BEEPER_INIT 3
#define BEEPER_CARRIAGE 2
#define BEEPER_NEXT_LINE 1

#define LED_SLOW_ON 500
#define LED_SLOW_OFF 500
#define LED_FAST_ON 100
#define LED_FAST_OFF 100

#define LED_FLASH_DURATION 200

//----------------------------------------------------------------------------
// Config class
//----------------------------------------------------------------------------

class Config {
 public:
  uint8_t startNeedle, stopNeedle;
  bool continuousReporting;
  bool valid;
};

//----------------------------------------------------------------------------
// Knitter class
//----------------------------------------------------------------------------

class Knitter : protected API {
 public:
  Knitter(hardwareAbstraction::HalInterface *hal);
  ~Knitter() = default;

  // Reset objet's states
  void reset();
  // Schedule knitter's state machine
  void schedule();

 private:
  // Detect addresses and type of both GPIO expanders
  void _detectGpioExpanders(hardwareAbstraction::HalInterface *hal, const uint8_t i2cAddress[][2], GpioExpander* gpio_expander[2]);
  // (Re)set carriage type/position and beltshift when crossing one sensor
  void _checkHallSensors();
  // Set solenoids based on current machine state
  void _runMachine();

  // Derived class method indicating Rx traffic
  void _apiRxTrafficIndication() override;
  // Derived class method indicating Tx traffic
  void _apiTxTrafficIndication() override;

  //----------------------------------------------------------------------------
  // RX message handlers
  //----------------------------------------------------------------------------

  // Call derived class method to reset the machine
  void _apiRequestReset() override;
  // Derived class method to set the machine type
  ErrorCode _apiRequestInit(MachineType machine) override;
  // Derived class method to set the machine's configuration
  ErrorCode _apiRxSetConfig(uint8_t startNeedle, uint8_t stopNeedle,
                            bool continuousReporting,
                            bool beeperEnabled) override;
  // Derived class method to set the line pattern
  ErrorCode _apiRxSetLine(uint8_t lineNumber, const uint8_t *pattern,
                          uint8_t size, bool isLastLine) override;
  // Call derived class method to report knitter state
  void _apiRxIndicateState() override;

  // Platform
  hardwareAbstraction::HalInterface *_hal;

  // Ayab Hardware
  Beeper *_beeper;
  Encoder *_encoder;
  HallSensor *_hall_left, *_hall_right;
  Led *_led_a, *_led_b;
  Solenoids *_solenoids;

  // Knitter objets
  Machine *_machine;
  Carriage *_carriage;
  BeltShift _beltShift;
  Direction _direction;

  // Ayab Firmware
  KnitterState _state, _lastState;
  Config _config;
  Line _currentLine;
  bool _resetFromOperate;
};

#endif