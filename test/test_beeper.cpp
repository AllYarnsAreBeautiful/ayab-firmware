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
  }

  void TearDown() override {
    releaseArduinoMock();
  }

  void checkBeepTime(uint8_t length) {
    EXPECT_CALL(*arduinoMock, analogWrite(PIEZO_PIN, BEEP_ON_DUTY)).Times(length);
    EXPECT_CALL(*arduinoMock, analogWrite(PIEZO_PIN, BEEP_OFF_DUTY)).Times(length);
    EXPECT_CALL(*arduinoMock, delay(BEEP_DELAY)).Times(length * 2);
    EXPECT_CALL(*arduinoMock, analogWrite(PIEZO_PIN, BEEP_NO_DUTY)).Times(1);
  }

  ArduinoMock *arduinoMock;
};

TEST_F(BeeperTest, test_ready) {
  checkBeepTime(BEEP_NUM_READY);
  beeper->ready();
}

TEST_F(BeeperTest, test_finishedLine) {
  checkBeepTime(BEEP_NUM_FINISHEDLINE);
  beeper->finishedLine();
}

TEST_F(BeeperTest, test_endWork) {
  checkBeepTime(BEEP_NUM_ENDWORK);
  beeper->endWork();
}
