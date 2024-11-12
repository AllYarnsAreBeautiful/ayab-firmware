#include <string.h>

#include "line.h"

#define CARRIAGE_OVERFLOW 11

void Line::reset() {
  memset(_pattern, 0xff, sizeof(_pattern));
  _number = 0xff;  // First getNextLineNumber is 0
  _isLast = false;
  _isWorking = false;
  _enterSide = MachineSide::None;
  requested = false;
  finished = true;
}

void Line::workedOn(MachineSide side, Direction direction) {

  if (_isWorking) {
    // Exit direction at the opposite end
    if ((_enterSide == MachineSide::Left) && (side == MachineSide::Right)) {
      _carriageOverflow = (direction == Direction::Right) ? _carriageOverflow-1 : _carriageOverflow+1;
    } else if ((_enterSide == MachineSide::Right) && (side == MachineSide::Left)) {
      _carriageOverflow = (direction == Direction::Left) ? _carriageOverflow-1 : _carriageOverflow+1;
    }
    // Extend by CARRIAGE_OVERFLOW needles to make secure last active ones
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
      _carriageOverflow = CARRIAGE_OVERFLOW;
      _isWorking = true;
    } else {
    // Register direction before entering the active needle window
      _enterSide = side;
    }
  }
}

bool Line::setPattern(uint8_t number, const uint8_t *line, bool isLast) {
  if (number == getNextLineNumber()) {
    memcpy(_pattern, line, sizeof(_pattern));
    _number++;
    _isLast = isLast;
    _isWorking = false;
    requested = false;
    finished = false;
    return true;
  }
  return false;
}

bool Line::getNeedleValue(int16_t position) {
  bool state = true;
  uint8_t byteIndex = position / 8;
  uint8_t bitIndex = position % 8;

  if ((byteIndex >= 0) && (byteIndex < sizeof(_pattern))) {
    // lsb = leftmost (check)
    state = _pattern[byteIndex] & (1 << bitIndex) ? false : true;
  }
  return state;
}

bool Line::isLastLine() { return _isLast; }

uint8_t Line::getNextLineNumber() { return _number + 1; }