#ifndef HAL_H
#define HAL_H

#include <stddef.h>
#include <stdint.h>

// digital pin levels
#define LOW 0x0
#define HIGH 0x1

// pin mode
#define INPUT 0x0
#define OUTPUT 0x1
#define INPUT_PULLUP 0x2

// interrupt mode
#define CHANGE 1

namespace hardwareAbstraction {

    class PacketSerialInterface {
    public:
        virtual ~PacketSerialInterface() = default;

        virtual void send(const uint8_t *buffer, size_t size) = 0;
        virtual void setPacketHandler(void (*callback)(const uint8_t* buffer, size_t size)) = 0;
        virtual void schedule() = 0;
    };

    class I2cInterface {
    public:
        virtual ~I2cInterface() = default;

        virtual void write(uint8_t device, uint8_t address, uint8_t value) = 0;   
    };

    class HalInterface {
    public:
        virtual ~HalInterface() = default;

        virtual void pinMode(uint8_t pin, uint8_t mode) = 0;
        virtual void digitalWrite(uint8_t pin, uint8_t state) = 0;
        virtual int digitalRead(uint8_t pin) = 0;
        virtual void analogWrite(uint8_t pin, uint8_t value) = 0;
        virtual int analogRead(uint8_t pin) = 0;
        virtual unsigned long millis() = 0;
        virtual void delayMicroseconds(unsigned int us) = 0;
        virtual void attachInterrupt(uint8_t interruptNum, void (*userFunc)(), int mode) = 0;

        I2cInterface *i2c;
        PacketSerialInterface *packetSerial;
    };

} // hardwareAbstraction

#endif // HAL_H
