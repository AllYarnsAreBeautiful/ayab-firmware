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
  // Update carriage type & position when sensor is crossed in the requested direction
  bool isCrossing(HallSensor *sensor, Direction direction);
  // Check if machine type is defined
  bool isDefined();
  // Return current carriage type
  CarriageType getType();
  // Define carriage position
  void setPosition(uint8_t position);
  // Return current carriage position
  uint8_t getPosition();
  // Return the needle to select given current carriage state
  uint8_t getSelectPosition(Direction direction);

 private:
  CarriageType _type;
  uint8_t _position;
};

#endif