#ifndef ENCODER_H
#define ENCODER_H

#include "hal.h"

class Encoder {
 public:
  Encoder(hardwareAbstraction::HalInterface *hal, uint8_t pin_v1,
          uint8_t pin_v2);
  ~Encoder() = default;

  // ISR called upon v1/encA change
  void interruptHandler();
  // Return current encoder position
  int16_t getPosition();
  // Define encoder position
  void setPosition(int16_t position);
  // Update internal variables/state machine
  void schedule();

 private:
  hardwareAbstraction::HalInterface *_hal;
  uint8_t _pin_v1, _pin_v2;
  int16_t _position, _isr_position;
  bool _isr_doorbell;
};

#endif