#include "beeper.h"

constexpr unsigned long BEEP_TIME_ON = 100;
constexpr unsigned long BEEP_TIME_OFF = 100;
constexpr uint8_t BEEP_VALUE_ON = 32;
constexpr uint8_t BEEP_VALUE_OFF = 255;

Beeper::Beeper(hardwareAbstraction::HalInterface *hal, uint8_t pin) {
  _hal = hal;
  _pin = pin;
  _state = State::Idle;
  _enabled = true;

  _off();
  _hal->pinMode(_pin, OUTPUT);
}

// TODO: Verify if PWM is really required vs simple on/off for all HW
void Beeper::_on() {
  _hal->analogWrite(_pin, BEEP_VALUE_ON);
}

void Beeper::_off() {
// WA: UNO R4 analogWrite generates a 255/256 PWM signal iso 255/255=HIGH
#if defined(ARDUINO_UNOR4_WIFI) || defined(ARDUINO_UNOR4_MINIMA)
  _hal->digitalWrite(_pin, HIGH);
#else
  _hal->analogWrite(_pin, BEEP_VALUE_OFF);
#endif
}

void Beeper::beep(uint8_t number) {
  if (_enabled && !busy()) {
    _number = number;
    _state = State::On;
  }
}

void Beeper::config(bool beeperEnabled) { _enabled = beeperEnabled; }

bool Beeper::busy() { return _state != State::Idle; }

void Beeper::schedule() {
  switch (_state) {
    case State::On:
      _on();
      _state = State::Wait;
      _nextState = State::Off;
      _nextTime = _hal->millis() + BEEP_TIME_ON;
      break;

    case State::Off:
      _off();
      _state = State::Wait;
      _nextState = State::On;
      _nextTime = _hal->millis() + BEEP_TIME_OFF;
      _number--;
      break;

    case State::Wait:
      if (_hal->millis() > _nextTime) {
        if (_number == 0) {
          _off();
          _state = State::Idle;
        } else {
          _state = _nextState;
        }
      }
      break;

    case State::Idle:
    default:
      break;
  }
}
