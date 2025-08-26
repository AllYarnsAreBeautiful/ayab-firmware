#ifndef I2C_H
#define I2C_H

#include "hal.h"

namespace hardwareAbstraction {

class I2c : public I2cInterface {
 public:
  I2c();
  ~I2c() = default;

  bool detect(uint8_t device) override;
  uint8_t read(uint8_t device, uint8_t address);
  void write(uint8_t device, uint8_t value) override;
  void write(uint8_t device, uint8_t address, uint8_t value) override;
};
}  // namespace hardwareAbstraction

#endif