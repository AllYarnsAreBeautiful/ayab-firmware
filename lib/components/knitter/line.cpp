#include <string.h>

#include "line.h"

void Line::reset() {
  memset(_pattern, 0xff, sizeof(_pattern));
  _number = 0xff;  // First getNextLineNumber is 0
  _isLast = false;
  requested = false;
  finished = true;
}

bool Line::setPattern(uint8_t number, const uint8_t *line, uint8_t size,
                      bool isLast) {
  if ((number != getNextLineNumber()) || (line == nullptr) || (size == 0) ||
      (size > sizeof(_pattern))) {
    return false;
  }

  // KH270 rows contain 14 bytes while standard-gauge rows contain 25.
  // Reset the unused bytes before copying the received row so a short row
  // cannot retain selections from a previous pattern.
  memset(_pattern, 0xff, sizeof(_pattern));
  memcpy(_pattern, line, size);
  _number++;
  _isLast = isLast;
  requested = false;
  finished = false;
  return true;
}

bool Line::getNeedleValue(int16_t position) {
  if ((position < 0) || (position >= (int16_t)(sizeof(_pattern) * 8))) {
    return true;
  }

  uint8_t byteIndex = position / 8;
  uint8_t bitIndex = position % 8;
  // lsb = leftmost (check)
  return (_pattern[byteIndex] & (1 << bitIndex)) == 0;
}

bool Line::isLastLine() { return _isLast; }

uint8_t Line::getNextLineNumber() { return _number + 1; }
