#include "gpio_expander.h"
#include "api.h"
#include "crc8.h"

GpioExpander::GpioExpander(hardwareAbstraction::HalInterface *hal, const uint8_t i2cAddress) {
  _hal = hal;
  _i2cAddress = i2cAddress;
  _cache_invalid = true;
}

void DummyExpander::update(uint8_t value) {
  uint8_t message[] = {(uint8_t)AYAB_API::debugPrint, (uint8_t)debugPrintMessageType::i2cWrite, (uint8_t)_i2cAddress, (uint8_t)value, 0};
  size_t size = sizeof(message);
  message[size - 1] = crc8(message, size - 1);
  _hal->packetSerial->send(message, size);
}
