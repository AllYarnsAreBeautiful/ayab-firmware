#include "encoder.h"
#include "atomic.h"

Encoder *encoderInstance;
void staticInterruptHandler() { encoderInstance->interruptHandler(); }

Encoder::Encoder(hardwareAbstraction::HalInterface *hal, uint8_t pin_v1,
                 uint8_t pin_v2) {
  _hal = hal;
  _pin_v1 = pin_v1;
  _pin_v2 = pin_v2;

  _hal->pinMode(_pin_v1, INPUT);
  _hal->pinMode(_pin_v2, INPUT);

  _position = 0;
  _isr_doorbell = false;
  _isr_position = 0;

  encoderInstance = this;

  _hal->attachInterrupt(_pin_v1, staticInterruptHandler, CHANGE);
}

void Encoder::interruptHandler() {
  // Update state when V2 is high
  if (_hal->digitalRead(_pin_v2) == HIGH) {
    _isr_doorbell = true;
    if (_hal->digitalRead(_pin_v1) == HIGH) {
      // Rising edge
      _isr_position += 1;
    } else {
      // Falling edge
      _isr_position -= 1;
    }
  }
}

void Encoder::setPosition(int16_t position) {
  ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
    _isr_position = position;
    _position = position;
  }
}

int16_t Encoder::getPosition() { return _position; }

void Encoder::schedule() {
  if (_isr_doorbell) {
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
      _isr_doorbell = false;
      _position = _isr_position;
    }
  }
}