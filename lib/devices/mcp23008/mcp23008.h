#ifndef MCP23008_H
#define MCP23008_H

#include "hal.h"
#include "../gpio_expander/gpio_expander.h"

#define MCP23008_IODIR 0x00
#define MCP23008_IOCON 0x05
#define MCP23008_OLAT 0x0a

class Mcp23008 final : public GpioExpander {
 public:
  Mcp23008(hardwareAbstraction::HalInterface *hal, uint8_t i2cAddress)
      : GpioExpander(hal, i2cAddress) {};
  ~Mcp23008() = default;

  // Update output latch register
  void update(uint8_t value) override;
  // Write a MCP23008 register
  void write(uint8_t reg, uint8_t value);
};

#endif