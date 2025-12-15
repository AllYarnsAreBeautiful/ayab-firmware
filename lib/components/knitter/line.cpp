#include <string.h>

#include "line.h"

void Line::reset() {
  memset(_pattern, 0xff, sizeof(_pattern));
  _number = 0xff;  // First getNextLineNumber is 0
  _isLast = false;
  requested = false;
  finished = true;
}

bool Line::setPattern(uint8_t number, const uint8_t *line, bool isLast) {
  if (number == getNextLineNumber()) {
    memcpy(_pattern, line, sizeof(_pattern));
    _number++;
    _isLast = isLast;
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