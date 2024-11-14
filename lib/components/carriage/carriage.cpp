#include "carriage.h"

Carriage::Carriage() { reset(); }

void Carriage::reset() {
  _type = CarriageType::NoCarriage;
  _position = 0;  // doesn't matter a priori
  _isWorking = false;
  _enterSide = MachineSide::None;
}

bool Carriage::workFinished(MachineSide side, Direction direction) {

  bool finished = false;

  if (_isWorking) {
    // Exit direction at the opposite end
    if ((_enterSide == MachineSide::Left) && (side == MachineSide::Right)) {
      _carriageOverflow = (direction == Direction::Right) ? _carriageOverflow-1 : _carriageOverflow+1;
    } else if ((_enterSide == MachineSide::Right) && (side == MachineSide::Left)) {
      _carriageOverflow = (direction == Direction::Left) ? _carriageOverflow-1 : _carriageOverflow+1;
    }
    // Extend carriage travel to make secure turnaround
    if (_carriageOverflow == 0) {
      finished = true;
      _isWorking = false;
      _enterSide = MachineSide::None;
    }
  } else {
    if (side == MachineSide::None) {
      // Handle case were active window is already entered when carriage is detected
      if (_enterSide == MachineSide::None) {
        _enterSide = (direction == Direction::Right) ? MachineSide::Left : MachineSide:: Right;
      }
      _carriageOverflow = getExtension();
      _isWorking = true;
    } else {
      // Register direction before entering the active needle window
      _enterSide = side;
    }
  }

  return finished;
}

bool Carriage::isCrossing(HallSensor *sensor, Direction requestedDirection) {
  // offset = # of needles elapsed since detection
  int16_t offset = _position - sensor->getDetectedPosition();
  // direction derived from offset
  Direction direction = (offset > 0) ? Direction::Right : Direction::Left;

  // Update carriage type & position if sensor is passed in the requested direction
  // and type is not KH270 (only detected once - FIXME: remove this constraint)
  if ((direction == requestedDirection) && (_type != CarriageType::Knit270)) {
    reset();
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

uint8_t Carriage::getExtension() {
  // Minimum extension is 8 needles (distance from last solenoid selection
  // to corresponding needle selection), 16 for G (8 more needles have
  // to pass to reselect the same one in the other direction) and 6 for K270
  switch (_type) {
    case CarriageType::Garter:
      return 18; // 16 + 2 needles margin
      break;
    case CarriageType::Lace:
      return 12; // 8 + 4 (18mm/72mm [4+12] after needle selection/carriage center)
      break;
    case CarriageType::Knit270:
      return 8; // 6 + 2 (54mm/153mm [9+8] after needle selection/carriage center) FIXME: TBC
      break;
    default:  // CarriageType::Knit
      return 12; // 8 + 4 (18mm/126mm [16+12] after needle selection/carriage center)
      break;
  }    
}

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