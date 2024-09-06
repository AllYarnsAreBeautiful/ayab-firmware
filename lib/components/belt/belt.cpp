#include "belt.h"

Belt::Belt(hardwareAbstraction::HalInterface *hal, uint8_t pin) {
  _hal = hal;
  _pin = pin;
  _shift = false;  // shouldn't matter

  _hal->pinMode(_pin, INPUT);
}

void Belt::schedule() {
  _shift = _hal->digitalRead(_pin);
}

bool Belt::getShift() {
  return _shift;
}