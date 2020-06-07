#include "serial_encoding_mock.h"
#include <serial_encoding.h>

static SerialEncodingMock *gSerialEncodingMock = NULL;
SerialEncodingMock *serialEncodingMockInstance() {
  if (!gSerialEncodingMock) {
    gSerialEncodingMock = new SerialEncodingMock();
  }
  return gSerialEncodingMock;
}

void releaseSerialEncodingMock() {
  if (gSerialEncodingMock) {
    delete gSerialEncodingMock;
    gSerialEncodingMock = NULL;
  }
}

SerialEncoding::SerialEncoding() {
}

void SerialEncoding::update() {
  assert(gSerialEncodingMock != nullptr);
  gSerialEncodingMock->update();
}

void SerialEncoding::send(uint8_t *payload, size_t length) {
  assert(gSerialEncodingMock != nullptr);
  gSerialEncodingMock->send(payload, length);
}

void SerialEncoding::onPacketReceived(const uint8_t *buffer, size_t size) {
  assert(gSerialEncodingMock != nullptr);
  gSerialEncodingMock->onPacketReceived(buffer, size);
}
