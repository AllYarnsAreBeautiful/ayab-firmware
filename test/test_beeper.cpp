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
