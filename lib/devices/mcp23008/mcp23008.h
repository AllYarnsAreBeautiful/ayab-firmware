#ifndef MCP23008_H
#define MCP23008_H

#include "hal.h"
#include "../gpio_expander/gpio_expander.h"

class Mcp23008 final : public GpioExpander {
 public:
  static constexpr uint8_t IODIR = 0x00;
  static constexpr uint8_t IOCON = 0x05;
  static constexpr uint8_t OLAT  = 0x0A;

  Mcp23008(hardwareAbstraction::HalInterface *hal, uint8_t i2cAddress)
      : GpioExpander(hal, i2cAddress) {};
  ~Mcp23008() = default;

  // Update output latch register
  void update(uint8_t value) override;
  // Write a MCP23008 register
  void write(uint8_t reg, uint8_t value);
};

#endif