#include "gtest/gtest.h"

#include "../knitter.h"
#include "knitter/beeper_mock.h"
#include "knitter/encoders_mock.h"
#include "knitter/solenoids_mock.h"

using ::testing::Return;

class KnitterTest : public ::testing::Test {
protected:
  void SetUp() override {
    arduinoMock = arduinoMockInstance();
    beeperMock = beeperMockInstance();
    solenoidsMock = solenoidsMockInstance();
    encodersMock = encodersMockInstance();
    k = Knitter(test_packetSerial);
  }

  void TearDown() override {
    releaseArduinoMock();
    releaseBeeperMock();
    releaseSolenoidsMock();
    releaseEncodersMock();
  }

  ArduinoMock *arduinoMock;
  BeeperMock *beeperMock;
  SolenoidsMock *solenoidsMock;
  EncodersMock *encodersMock;
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
  EXPECT_CALL(*encodersMock, encA_interrupt).Times(1);
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
