#include "gtest/gtest.h"

#define KH910
#include "../encoders.h"

TEST(encoders, mock_works) {
  ArduinoMock* arduinoMock = arduinoMockInstance();
  auto e = Encoders();
  EXPECT_CALL(*arduinoMock, digitalRead(ENC_PIN_A));
  e.encA_interrupt();
  releaseArduinoMock();
}
