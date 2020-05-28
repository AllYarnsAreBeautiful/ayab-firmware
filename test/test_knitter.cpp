#include "gtest/gtest.h"

#include "../knitter.h"

using ::testing::Return;

class KnitterTest : public ::testing::Test {
protected:
  void SetUp() override {
    arduinoMock = arduinoMockInstance();
    k = Knitter(test_packetSerial);
  }

  void TearDown() override {
    releaseArduinoMock();
  }

  ArduinoMock *arduinoMock;
  Knitter k;
  SLIPPacketSerial *test_packetSerial;
};

TEST(knitter_test, test_constructor) {
  Knitter k = Knitter();
  (void)k;
}

TEST_F(KnitterTest, test_constructor) {
}

TEST_F(KnitterTest, test_isr) {
  k.isr();
}

TEST_F(KnitterTest, test_fsm) {
  k.fsm();
}

TEST_F(KnitterTest, test_startOperation) {
  byte line[] = {1};
  k.startOperation(0, NUM_NEEDLES - 1, false, line);
}

TEST_F(KnitterTest, test_startTest) {
  k.startTest();
}

TEST_F(KnitterTest, test_setNextLine) {
  k.setNextLine(1);
}

TEST_F(KnitterTest, test_setLastLine) {
  k.setLastLine();
}
