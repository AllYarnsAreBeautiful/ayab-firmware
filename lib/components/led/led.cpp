#include "led.h"

Led::Led(hardwareAbstraction::HalInterface *hal, uint8_t pin)
    : Led(hal, pin, LOW, HIGH) {}

Led::Led(hardwareAbstraction::HalInterface *hal, uint8_t pin, uint8_t offValue, uint8_t onValue) {
  _hal = hal;
  _pin = pin;
  _offValue = offValue;
  _onValue = onValue;

  off();
  _hal->pinMode(_pin, OUTPUT);
}

void Led::_write() {
  if (_state == State::Off)
    _hal->digitalWrite(_pin, _offValue);
  else
    _hal->digitalWrite(_pin, _onValue);

  _lasttime = _hal->millis();
}

void Led::off() {
  _mode = Mode::Solid;
  _state = State::Off;
  _write();
}

void Led::on() {
  _mode = Mode::Solid;
  _state = State::On;
  _write();
}

void Led::toggle() {
  _state = (_state == State::On) ? State::Off : State::On;
  _write();
}

void Led::blink(unsigned long timeOff, unsigned long timeOn) {
  _timeOff = timeOff;
  _timeOn = timeOn;

  _mode = Mode::Blink;
  toggle();
}

void Led::flash(unsigned long timeOn) {
  _flashOn = timeOn;
  if (_mode == Mode::Flash) {
    // extend flash time
    _lasttime += timeOn;
  } else {
    _savedMode = _mode;
    _mode = Mode::Flash;
    toggle();
  }
}

void Led::schedule() {
  unsigned long elapsedTime = _hal->millis() - _lasttime;

  switch (_mode) {
    case Mode::Blink:
      if (_state == State::On) {
        if (elapsedTime >= _timeOn) toggle();
      } else {
        if (elapsedTime >= _timeOff) {
          toggle();
        }
      }
      break;
    case Mode::Flash:
      if (elapsedTime >= _flashOn) {
        _mode = _savedMode;
        toggle();
      }
      break;
    default:
      break;
  }
}
