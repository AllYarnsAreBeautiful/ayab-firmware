#include "platform.h"
#include "packetSerial/packetSerial.h"
#include "i2c/i2c.h"

namespace hardwareAbstraction {

    Platform::Platform() {
        packetSerial = new hardwareAbstraction::PacketSerial();
        i2c = new hardwareAbstraction::I2c();
    }

    void Platform::pinMode(uint8_t pin, uint8_t mode) {
        ::pinMode(pin, mode);
    }

    void Platform::digitalWrite(uint8_t pin, uint8_t state) {
        ::digitalWrite(pin, state);
    }

    int Platform::digitalRead(uint8_t pin) {
        return ::digitalRead(pin);
    }

    void Platform::analogWrite(uint8_t pin, uint8_t value) {
        ::analogWrite(pin, value);
    }

    int Platform::analogRead(uint8_t pin) {
        return ::analogRead(pin);
    }

    unsigned long Platform::millis() {
        return ::millis();
    }

    void Platform::attachInterrupt(uint8_t interruptNum, void (*userFunc)(), int mode) {
        ::attachInterrupt(digitalPinToInterrupt(interruptNum), userFunc, mode);
    }
} // hardwareAbstraction