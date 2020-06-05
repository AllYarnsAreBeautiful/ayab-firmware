#include <gtest/gtest.h>

#include <beeper.h>
#include <board.h>

using ::testing::Return;

class BeeperTest : public ::testing::Test {
protected:
  void SetUp() override {
    arduinoMock = arduinoMockInstance();
    b = Beeper();
  }

  void TearDown() override {
    releaseArduinoMock();
  }

  void checkBeepTime(uint8_t length) {
    EXPECT_CALL(*arduinoMock, analogWrite(PIEZO_PIN, 0)).Times(length);
    EXPECT_CALL(*arduinoMock, analogWrite(PIEZO_PIN, 20)).Times(length);
    EXPECT_CALL(*arduinoMock, delay(BEEP_DELAY)).Times(length * 2);
    EXPECT_CALL(*arduinoMock, analogWrite(PIEZO_PIN, 255)).Times(1);
  }

  ArduinoMock *arduinoMock;
  Beeper b;
};

TEST_F(BeeperTest, test_ready) {
  checkBeepTime(5);
  b.ready();
}

TEST_F(BeeperTest, test_finishedLine) {
  checkBeepTime(3);
  b.finishedLine();
}

TEST_F(BeeperTest, test_endWork) {
  checkBeepTime(10);
  b.endWork();
}
