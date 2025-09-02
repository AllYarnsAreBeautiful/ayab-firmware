#include "pcf8574.h"

void Pcf8574::update(uint8_t value) {
  if ((value == _output_latch_cache) && (!_cache_invalid)) {
    return;
  }
  _output_latch_cache = value;
  _cache_invalid = false;

  _hal->i2c->write(_i2cAddress, value);
}