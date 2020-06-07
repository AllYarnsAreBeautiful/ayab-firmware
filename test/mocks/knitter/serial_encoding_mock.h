#pragma once

#include <gmock/gmock.h>

class SerialEncodingMock {
public:
  MOCK_METHOD0(update, void());
  MOCK_METHOD2(send, void(uint8_t *payload, size_t length));
  MOCK_METHOD2(onPacketReceived, void(const uint8_t *buffer, size_t size));
};

SerialEncodingMock *serialEncodingMockInstance();
void releaseSerialEncodingMock();
