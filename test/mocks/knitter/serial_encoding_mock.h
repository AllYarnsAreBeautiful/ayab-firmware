#pragma once

#include <gmock/gmock.h>

class SerialEncodingMock {
public:
  MOCK_METHOD0(update, void());
  MOCK_METHOD2(send, void(uint8_t *payload, size_t length));
};

SerialEncodingMock *serialEncodingMockInstance();
void releaseSerialEncodingMock();
