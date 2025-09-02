#include "mcp23008.h"

void Mcp23008::update(uint8_t value) {
  write(MCP23008_OLAT, value);
}

void Mcp23008::write(uint8_t reg, uint8_t value) {
  if (reg == MCP23008_OLAT) {
    if ((value == _output_latch_cache) && (!_cache_invalid)) {
      return;
    }
    _output_latch_cache = value;
    _cache_invalid = false; // TODO: invalidate after write() failure when supported
  }

  _hal->i2c->write(_i2cAddress, reg, value);
}