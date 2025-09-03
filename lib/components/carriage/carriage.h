#ifndef CARRIAGE_H
#define CARRIAGE_H

#include <stdint.h>

#include "api.h"
#include "hallsensor.h"

class Carriage {
 public:
  Carriage();
  ~Carriage() = default;

  // Reset carriage state
  void reset();
  // Update carriage state and return true when work is finished 
  bool workFinished(MachineSide side, Direction direction);
  // Update carriage type & position when sensor is crossed in the requested direction
  bool isCrossing(HallSensor *sensor, Direction direction);
  // Check if machine type is defined
  bool isDefined();
  // Return current carriage type
  CarriageType getType();
  // Define carriage position
  void setPosition(int16_t position);
  // Return current carriage position
  int16_t getPosition();
  // Return travel extension to make secure turnaround
  uint8_t getExtension();
  // Return the needle to select given current carriage state
  int16_t getSelectPosition(Direction direction);

 private:
  CarriageType _type;
  int16_t _position;
  bool _isWorking;
  MachineSide _enterSide;
  uint8_t _carriageOverflow;
};

#endif