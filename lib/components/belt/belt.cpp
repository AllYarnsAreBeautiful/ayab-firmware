#include "belt.h"

Belt::Belt(hardwareAbstraction::HalInterface *hal, uint8_t pin) {
  _hal = hal;
  _pin = pin;
  _shift = false;  // shouldn't matter

  _hal->pinMode(_pin, INPUT);
}

void Belt::setshift(Direction direction, CarriageType carriageType) {
  _shift = _hal->digitalRead(_pin) == 0;
  if ((carriageType == CarriageType::Knit) && (direction == Direction::Right)) {
    // FIXME: K when moving to the right not super clear -> supply sensr label
    // instead ?
    _shift = !_shift;
  }
}

BeltShift Belt::getShift() {
  return _shift == 0 ? BeltShift::Regular : BeltShift::Shifted;
}