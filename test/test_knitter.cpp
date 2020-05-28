#include "gtest/gtest.h"

#include "../knitter.h"
#include "knitter/beeper_mock.h"
#include "knitter/encoders_mock.h"
#include "knitter/solenoids_mock.h"

using ::testing::Return;
void onPacketReceived(const uint8_t *buffer, size_t size) {
  (void)buffer;
  (void)size;
}

class KnitterTest : public ::testing::Test {
protected:
  void SetUp() override {
    arduinoMock = arduinoMockInstance();
    beeperMock = beeperMockInstance();
    solenoidsMock = solenoidsMockInstance();
    encodersMock = encodersMockInstance();
    serialMock = serialMockInstance();
    // test_packetSerial->begin(SERIAL_BAUDRATE);
    test_packetSerial.setPacketHandler(&onPacketReceived);
    k = Knitter(&test_packetSerial);
  }

  void TearDown() override {
    releaseArduinoMock();
    releaseBeeperMock();
    releaseSolenoidsMock();
    releaseEncodersMock();
    releaseSerialMock();
  }

  ArduinoMock *arduinoMock;
  BeeperMock *beeperMock;
  SolenoidsMock *solenoidsMock;
  EncodersMock *encodersMock;
  SerialMock *serialMock;
  Knitter k;
  SLIPPacketSerial test_packetSerial;
};

TEST(knitter_test, test_constructor) {
  Knitter k = Knitter();
  (void)k;
}

TEST_F(KnitterTest, test_constructor) {
}

TEST_F(KnitterTest, test_isr) {
  EXPECT_CALL(*encodersMock, encA_interrupt).Times(1);
  EXPECT_CALL(*encodersMock, getPosition).Times(1);
  EXPECT_CALL(*encodersMock, getDirection).Times(1);
  EXPECT_CALL(*encodersMock, getHallActive).Times(1);
  EXPECT_CALL(*encodersMock, getBeltshift).Times(1);
  EXPECT_CALL(*encodersMock, getCarriage).Times(1);
  k.isr();
}

TEST_F(KnitterTest, test_fsm) {
  // Machine is initialized when left hall sensor is passed in Right direction
  EXPECT_CALL(*encodersMock, encA_interrupt).Times(1);
  EXPECT_CALL(*encodersMock, getPosition).Times(1);
  EXPECT_CALL(*encodersMock, getDirection).Times(2).WillOnce(Return(Right));
  EXPECT_CALL(*encodersMock, getHallActive).Times(1).WillOnce(Return(Left));
  EXPECT_CALL(*encodersMock, getBeltshift).Times(1);
  EXPECT_CALL(*encodersMock, getCarriage).Times(1);
  k.isr();
  EXPECT_CALL(*solenoidsMock, setSolenoids(0xFFFF)).Times(1);
  // init to ready
  k.fsm();
  // ready
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
