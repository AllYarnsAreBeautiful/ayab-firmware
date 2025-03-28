#include <Arduino.h>
#include <PacketSerial.h>

#include "hal.h"

namespace hardwareAbstraction {

class PacketSerial : public PacketSerialInterface {
 public:
  PacketSerial();
  ~PacketSerial() = default;

  void send(const uint8_t* buffer, size_t size) override;
  void setPacketHandler(void (*callback)(const uint8_t* buffer,
                                         size_t size)) override;
  void schedule() override;

 private:
  SLIPPacketSerial myPacketSerial;
};
}  // namespace hardwareAbstraction