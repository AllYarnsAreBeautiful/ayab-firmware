#include "pcf8574.h"

void Pcf8574::write(uint8_t address, uint8_t value) {
  if ((value == _olat_cache) && (!_olat_cache_invalid)) {
    return;
  }
  _olat_cache = value;
  _olat_cache_invalid = false;

  _hal->i2c->write(_i2cAddress, value);
}