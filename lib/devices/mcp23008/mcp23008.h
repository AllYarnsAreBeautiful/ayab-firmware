#ifndef MCP23008_H
#define MCP23008_H

#include "hal.h"

#define MCP23008_IODIR 0x00
#define MCP23008_OLAT 0x0a

class Mcp23008 {
 public:
  Mcp23008(hardwareAbstraction::HalInterface *hal, const uint8_t i2cAddress);
  ~Mcp23008() = default;

  // Write a MCP23008 register
  void write(uint8_t address, uint8_t value);

 private:
  hardwareAbstraction::HalInterface *_hal;
  uint8_t _i2cAddress;
  uint8_t _olat_cache;
  bool _olat_cache_invalid;
};

#endif