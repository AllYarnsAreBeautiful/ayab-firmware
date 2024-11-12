#include "carriage.h"

Carriage::Carriage() { reset(); }

void Carriage::reset() {
  _type = CarriageType::NoCarriage;
  _position = 0;  // doesn't matter a priori
}

bool Carriage::isCrossing(HallSensor *sensor, Direction requestedDirection) {
  // offset = # of needles elapsed since detection
  int16_t offset = _position - sensor->getDetectedPosition();
  // direction derived from offset
  Direction direction = (offset > 0) ? Direction::Right : Direction::Left;

  // Update carriage type & position if sensor is passed in the requested direction
  // and type is not KH270 (only detected once - FIXME: remove this constraint)
  if ((direction == requestedDirection) && (_type != CarriageType::Knit270)) {
    _type = sensor->getDetectedCarriage();
    _position = (int16_t) sensor->getSensorPosition() + offset;
    if (_type == CarriageType::Garter) {
      // Inner magnets are +/-12 needles from the center
      _position = direction == Direction::Left ? _position  + 12 : _position - 12;
     } else if (_type == CarriageType::Knit270) {
      // Inner magnets are +/-3 needles from the center, sensors are at -3 and 114
      _position = direction == Direction::Left ? _position  + 6 : _position - 6;
    }
    return true;
  }
  return false;
}

bool Carriage::isDefined() { return _type != CarriageType::NoCarriage; }

CarriageType Carriage::getType() { return _type; }

void Carriage::setPosition(int16_t position) { _position = position; }

int16_t Carriage::getPosition() { return _position; }

int16_t Carriage::getSelectPosition(Direction direction) {
  // Selection you take place 12 before, 4 after the needle checker (NC)
  // position
  switch (_type) {
    case CarriageType::Garter:
      // G: NC @ +0/Left, -0/Right and magnets @-12 and +12
      // solenoid selection @ -8/Left, +8/Right 
      return (direction == Direction::Left) ? _position - 8 : _position + 8;
      break;
    case CarriageType::Lace:
      // L: NC @ +12/Left, -12/Right and magnet @ 0
      // solenoid selection @ +4/Left, -4/Right       
      return (direction == Direction::Left) ? _position + 4 : _position - 4;
      break;
    case CarriageType::Knit270:
      // K on KH270: NC @ +12/Left, -12/Right and magnets @-3 and +3
      // solenoid selection @ +6/Left, -6/Right       
      return (direction == Direction::Left) ? _position + 6 : _position - 6;
      break;
    default:  // CarriageType::Knit
      // K: NC @ +24/Left, -24/Right and magnet @ 0
      // solenoid selection @ +16/Left, -16/Right
      return (direction == Direction::Left) ? _position + 16: _position - 16;
      break;
  }
}