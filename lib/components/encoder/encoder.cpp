#include "encoder.h"
#include "atomic.h"

Encoder *encoderInstance;
void staticInterruptHandler() { encoderInstance->interruptHandler(); }

Encoder::Encoder(hardwareAbstraction::HalInterface *hal, uint8_t pin_v1,
                 uint8_t pin_v2) {
  _hal = hal;
  _pin_v1 = pin_v1;
  _pin_v2 = pin_v2;

  _pin_v1_previous_state = UNDEFINED;

  _hal->pinMode(_pin_v1, INPUT);
  _hal->pinMode(_pin_v2, INPUT);

  _position = 0;
  _isr_doorbell = false;
  _isr_position = 0;

  encoderInstance = this;

  _hal->attachInterrupt(_pin_v1, staticInterruptHandler, CHANGE);
}

void Encoder::interruptHandler() {
  // Read current V1 & V2 states
  uint8_t pin_v1_current_state = _hal->digitalRead(_pin_v1);
  uint8_t pin_v2_current_state = _hal->digitalRead(_pin_v2);

  if (_pin_v1_previous_state == pin_v1_current_state) {
    // Spurious interrupt, ignore
    return;
  }

  // Update state when V2 is high
  if (pin_v2_current_state == HIGH) {
    _isr_doorbell = true;
    if (pin_v1_current_state == HIGH) {
      // Rising edge
      _isr_position += 1;
    } else {
      // Falling edge
      _isr_position -= 1;
    }
  }

  _pin_v1_previous_state = pin_v1_current_state;
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
