#ifndef PLATFORM_H
#define PLATFORM_H

#include <Arduino.h>

#include "hal.h"

namespace hardwareAbstraction {

class Platform : public HalInterface {
 public:
  Platform();
  ~Platform() = default;

  void pinMode(uint8_t pin, uint8_t mode) override;
  void digitalWrite(uint8_t pin, uint8_t state) override;
  int digitalRead(uint8_t pin) override;
  void analogWrite(uint8_t pin, uint8_t value) override;
  int analogRead(uint8_t pin) override;
  unsigned long millis() override;
  void delayMicroseconds(unsigned int us) override;
  void attachInterrupt(uint8_t interruptNum, void (*userFunc)(),
                       int mode) override;
};
}  // namespace hardwareAbstraction

#endif  // PLATFORM_H
