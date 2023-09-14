/*!`
 * \file test_beeper.cpp
 *
 * This file is part of AYAB.
 *
 *    AYAB is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation, either version 3 of the License, or
 *    (at your option) any later version.
 *
 *    AYAB is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with AYAB.  If not, see <http://www.gnu.org/licenses/>.
 *
 *    Original Work Copyright 2013 Christian Obersteiner, Andreas Müller
 *    Modified Work Copyright 2020 Sturla Lange, Tom Price
 *    http://ayab-knitting.com
 */

#include <gtest/gtest.h>

#include <beeper.h>
#include <board.h>

using ::testing::Return;

extern Beeper *beeper;

class BeeperTest : public ::testing::Test {
protected:
  void SetUp() override {
    arduinoMock = arduinoMockInstance();

    // start in BeepState::Idle
    beeper->init();
  }

  void TearDown() override {
    releaseArduinoMock();
  }

  ArduinoMock *arduinoMock;

  void expectedBeepSchedule(unsigned long t) {
    EXPECT_CALL(*arduinoMock, millis).WillOnce(Return(t));
    beeper->schedule();
  }

  void expectedBeepRepeats(uint8_t repeats) {
      ASSERT_EQ(beeper->getState(), BeepState::Wait);
    for (uint8_t i = 0; i < repeats; i++) {
      expectedBeepSchedule(BEEP_DELAY * 2 * i);
      EXPECT_CALL(*arduinoMock, analogWrite(PIEZO_PIN, BEEP_ON_DUTY));
      expectedBeepSchedule(BEEP_DELAY * 2 * i + 1);
      expectedBeepSchedule(BEEP_DELAY * (2 * i + 1));
      //ASSERT_EQ(beeper->getState(), BeepState::Off);
      EXPECT_CALL(*arduinoMock, analogWrite(PIEZO_PIN, BEEP_OFF_DUTY));
      expectedBeepSchedule(BEEP_DELAY * (2 * i + 1) + 1);
    }
    expectedBeepSchedule(BEEP_DELAY * (2 * repeats));
    EXPECT_CALL(*arduinoMock, analogWrite(PIEZO_PIN, BEEP_NO_DUTY));
    expectedBeepSchedule(BEEP_DELAY * (2 * repeats) + 1);
    ASSERT_EQ(beeper->getState(), BeepState::Idle);
  }
};

TEST_F(BeeperTest, test_ready) {
  EXPECT_CALL(*arduinoMock, millis).WillOnce(Return(0U));
  beeper->ready();
  expectedBeepRepeats(BEEP_NUM_READY);
}

TEST_F(BeeperTest, test_finishedLine) {
  EXPECT_CALL(*arduinoMock, millis).WillOnce(Return(0U));
  beeper->finishedLine();
  expectedBeepRepeats(BEEP_NUM_FINISHEDLINE);
}

TEST_F(BeeperTest, test_endWork) {
  EXPECT_CALL(*arduinoMock, millis).WillOnce(Return(0U));
  beeper->endWork();
  expectedBeepRepeats(BEEP_NUM_ENDWORK);
}
