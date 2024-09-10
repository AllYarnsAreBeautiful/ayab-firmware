#include <string.h>

#include "line.h"

#define CARRIAGE_OVERFLOW 16

void Line::reset() {
  memset(_pattern, 0xff, sizeof(_pattern));
  _number = 0xff;  // First getNextLineNumber is 0
  _isLast = false;
  _enterDirection = Direction::Unknown;
  requested = false;
  finished = true;
}

void Line::workedOn(bool isWorkedOn, Direction direction) {
  if (isWorkedOn) {
    // Register direction when entering the active needle window
    if (_enterDirection == Direction::Unknown) {
      _enterDirection = direction;
      _carriageOverflow = CARRIAGE_OVERFLOW;
    }
  } else {
    // Exit direction has to match start direction (other end)
    if (_enterDirection == direction) {
      // Extend by CARRIAGE_OVERFLOW needles to make secure last active ones
      if (--_carriageOverflow == 0) {
        finished = true;
      }
    }
  }
}

bool Line::setPattern(uint8_t number, const uint8_t *line, bool isLast) {
  if (number == getNextLineNumber()) {
    memcpy(_pattern, line, sizeof(_pattern));
    _number++;
    _isLast = isLast;
    _enterDirection = Direction::Unknown;
    requested = false;
    finished = false;
    return true;
  }
  return false;
}

bool Line::getNeedleValue(uint8_t position) {
  bool state = true;
  uint8_t byteIndex = position >> 3;
  uint8_t bitIndex = position % 8;

  if ((byteIndex < sizeof(_pattern))) {
    // lsb = leftmost (check)
    state = _pattern[byteIndex] & (1 << bitIndex) ? false : true;
  }
  return state;
}

bool Line::isLastLine() { return _isLast; }

uint8_t Line::getNextLineNumber() { return _number + 1; }