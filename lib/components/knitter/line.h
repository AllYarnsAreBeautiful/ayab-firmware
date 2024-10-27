#ifndef LINE_H
#define LINE_H

#include "api.h"

class Line {
 public:
  Line() = default;
  ~Line() = default;

  // Reset objet's state
  void reset();

  // Indicate if line is worked on
  void workedOn(bool isWorkedOn, Direction direction);
  // Set a new line pattern
  bool setPattern(uint8_t number, const uint8_t *pattern, bool isLast);
  // Get the needle value for the given position
  bool getNeedleValue(int16_t position);
  // Return the next line number
  uint8_t getNextLineNumber();
  // Return true if the current line is the last one
  bool isLastLine();

  bool requested;  // setter/getter ?
  bool finished;   // setter/getter ?

 private:
  uint8_t _number;
  uint8_t _pattern[25];
  bool _isLast;
  Direction _enterDirection;
  uint8_t _carriageOverflow;
};

#endif