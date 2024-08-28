#include "packetSerial.h"

namespace hardwareAbstraction {

    PacketSerial::PacketSerial() {
        ::Serial.begin(115200);
        myPacketSerial.setStream(&::Serial);
    }

    void PacketSerial::send(const uint8_t *buffer, size_t size) {
        myPacketSerial.send(buffer, size);
    }

    void PacketSerial::setPacketHandler(void (*callback)(const uint8_t* buffer, size_t size)) {
        myPacketSerial.setPacketHandler(callback);
    }

    void PacketSerial::schedule() {
        myPacketSerial.update();
    }
}