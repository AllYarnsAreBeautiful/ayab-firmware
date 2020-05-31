#include "gtest/gtest.h"

#include "../knitter.h"

using ::testing::Return;

class KnitterTest : public ::testing::Test {
protected:
  void SetUp() override {
    arduinoMock = arduinoMockInstance();
    k = Knitter();
  }

  void TearDown() override {
    releaseArduinoMock();
  }

  ArduinoMock *arduinoMock;
  Knitter k;
};

TEST_F(KnitterTest, test) {
}
