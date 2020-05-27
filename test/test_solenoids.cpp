#include "gtest/gtest.h"

#include "../solenoids.h"

using ::testing::Return;

class SolenoidsTest : public ::testing::Test {
protected:
  void SetUp() override {
    arduinoMock = arduinoMockInstance();
    s = Solenoids();
  }

  void TearDown() override {
    releaseArduinoMock();
  }

  ArduinoMock *arduinoMock;
  Solenoids s;
};

TEST_F(SolenoidsTest, test_construct) {
}

// TODO(sl): The following tests cause a segfault, why?
// TEST_F(SolenoidsTest, test_init) {
//     s.init();
// }
//
// TEST_F(SolenoidsTest, test_setSolenoid) {
//     s.setSolenoid(1, true);
// }
//
// TEST_F(SolenoidsTest, test_setSolenoids) {
//     s.setSolenoids(0xFFFF);
// }
