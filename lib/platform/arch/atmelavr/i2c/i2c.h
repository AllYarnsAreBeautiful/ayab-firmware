#ifndef I2C_H
#define I2C_H

#include "hal.h"

namespace hardwareAbstraction {

class I2c : public I2cInterface {
 public:
  I2c();
  ~I2c() = default;

  void write(uint8_t device, uint8_t address, uint8_t value) override;
};
}  // namespace hardwareAbstraction

#endif