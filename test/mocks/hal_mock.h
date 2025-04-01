#ifndef HAL_MOCK_H
#define HAL_MOCK_H

#include "gmock/gmock.h"

#include "hal.h"

class PacketSerial_mock : public hardwareAbstraction::PacketSerialInterface {
public:
    MOCK_METHOD(void, send, (const uint8_t *buffer, size_t size), (override));
    MOCK_METHOD(void, setPacketHandler, (void (*callback)(const uint8_t* buffer, size_t size)), (override));
    MOCK_METHOD(void, schedule, (), (override));
};

class I2c_mock : public hardwareAbstraction::I2cInterface {
public:
    MOCK_METHOD(void, write,(uint8_t device, uint8_t address, uint8_t value), (override));
};

class Hal_mock : public hardwareAbstraction::HalInterface {
public:
    ~Hal_mock() override = default;

    MOCK_METHOD(void, pinMode, (uint8_t pin, uint8_t mode), (override));
    MOCK_METHOD(void, digitalWrite, (uint8_t pin, uint8_t state), (override));
    MOCK_METHOD(int, digitalRead, (uint8_t pin), (override));
    MOCK_METHOD(void, analogWrite, (uint8_t pin, uint8_t value), (override));
    MOCK_METHOD(int, analogRead, (uint8_t pin), (override));
    MOCK_METHOD(unsigned long, millis, (), (override));
    MOCK_METHOD(void, attachInterrupt, (uint8_t interruptNum, void (*userFunc)(), int mode), (override));

    I2c_mock *i2c;
    PacketSerial_mock *packetSerial;
};

#endif // HAL_MOCK_H