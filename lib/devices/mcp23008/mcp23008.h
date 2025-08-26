#ifndef MCP23008_H
#define MCP23008_H

#include "hal.h"
#include "../gpio_expander/gpio_expander.h"

#define MCP23008_IODIR 0x00
#define MCP23008_IOCON 0x05
#define MCP23008_OLAT 0x0a

class Mcp23008 : public GpioExpander {
 public:
  Mcp23008(hardwareAbstraction::HalInterface *hal, uint8_t i2cAddress): GpioExpander(hal, i2cAddress) {};
  ~Mcp23008() = default;

  // Write a MCP23008 register
  void write(uint8_t address, uint8_t value) override;
};

#endif