#pragma once

#include <encoders.h>
#include <gmock/gmock.h>

class EncodersMock {
public:
  MOCK_METHOD0(encA_interrupt, void());
  MOCK_METHOD0(getPosition, uint8_t());
  MOCK_METHOD0(getBeltshift, Beltshift_t());
  MOCK_METHOD0(getDirection, Direction_t());
  MOCK_METHOD0(getHallActive, Direction_t());
  MOCK_METHOD0(getCarriage, Carriage_t());
  MOCK_METHOD1(getHallValue, uint16_t(Direction_t));
};

EncodersMock *encodersMockInstance();
void releaseEncodersMock();
