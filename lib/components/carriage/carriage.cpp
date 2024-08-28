#include "carriage.h"

Carriage::Carriage() { reset(); }

void Carriage::reset() {
  _type = CarriageType::NoCarriage;
  _position = 0;  // doesn't matter a priori
}

bool Carriage::isCrossing(HallSensor *sensor, Direction requestedDirection) {
  // offset = # of needles elapsed since detection
  uint8_t offset = (uint8_t)(_position - sensor->getDetectedPosition());
  // direction derived from offset (manipulated as int8)
  Direction direction = (offset < 128) ? Direction::Right : Direction::Left;

  if (direction == requestedDirection) {
    _type = sensor->getDetectedCarriage();
    _position = sensor->getSensorPosition() + offset;
    return true;
  }
  return false;
}

bool Carriage::isDefined() { return _type != CarriageType::NoCarriage; }

CarriageType Carriage::getType() { return _type; }

void Carriage::setPosition(uint8_t position) { _position = position; }

uint8_t Carriage::getPosition() { return _position; }

uint8_t Carriage::getSelectPosition(Direction direction) {
  // Selection you take place 12 before, 4 after the needle checker (NC)
  // position
  switch (_type) {
    case CarriageType::G:
      // G: NC @ +0/Left, -0/Right => selection @ -12/Left, +12Left
      return (direction == Direction::Left) ? (uint8_t)(_position - 12)
                                            : (uint8_t)(_position + 12);
      break;
    case CarriageType::L:
      // L: NC @ +12/Left, -12/Right => selection @ +0/Left, -0/Left
      return (direction == Direction::Left) ? (uint8_t)(_position + 0)
                                            : (uint8_t)(_position - 0);
      break;
    default:  // CarriageType::K
      // K: NC @ +24/Left, -24/Right => selection @ +12/Left, -12/Left
      return (direction == Direction::Left) ? (uint8_t)(_position + 12)
                                            : (uint8_t)(_position - 12);
      break;
  }
}