#include "mcp23008.h"

void Mcp23008::write(uint8_t address, uint8_t value) {
  if (address == MCP23008_OLAT) {
    if ((value == _olat_cache) && (!_olat_cache_invalid)) {
      return;
    }
    _olat_cache = value;
    _olat_cache_invalid = false;
  }

  _hal->i2c->write(_i2cAddress, address, value);
}