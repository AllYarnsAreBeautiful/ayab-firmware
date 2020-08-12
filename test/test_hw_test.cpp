/*!`
 * \file test_hw_test.cpp
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

#include <SerialCommand_mock.h>
#include <hw_test.h>
#include <knitter_mock.h>

using ::testing::AtLeast;
using ::testing::Return;

static char zero[2] = {48, 0};       // "0"
static char two[2] = {50, 0};        // "2"
static char twenty[3] = {50, 48, 0}; // "20"

class HardwareTestTest : public ::testing::Test {
protected:
  void SetUp() override {
    arduinoMock = arduinoMockInstance();
    serialMock = serialMockInstance();
    serialCommandMock = serialCommandMockInstance();
    knitterMock = knitterMockInstance();
  }

  void TearDown() override {
    releaseArduinoMock();
    releaseSerialMock();
    releaseSerialCommandMock();
    releaseKnitterMock();
  }

  ArduinoMock *arduinoMock;
  SerialMock *serialMock;
  SerialCommandMock *serialCommandMock;
  KnitterMock *knitterMock;

  void expect_test() {
  }
};

TEST_F(HardwareTestTest, test_setUp) {
  EXPECT_CALL(*knitterMock, setQuitFlag);
  ASSERT_FALSE(HardwareTest::m_autoReadOn);
  ASSERT_FALSE(HardwareTest::m_autoTestOn);
  ASSERT_FALSE(HardwareTest::m_timerEvent);
  ASSERT_FALSE(HardwareTest::m_timerEventOdd);
  HardwareTest::setUp();
}

TEST_F(HardwareTestTest, test_helpCmd) {
  HardwareTest::helpCmd();
}

TEST_F(HardwareTestTest, test_sendCmd) {
  EXPECT_CALL(*knitterMock, send);
  HardwareTest::sendCmd();
}

TEST_F(HardwareTestTest, test_beepCmd) {
  EXPECT_CALL(*arduinoMock, analogWrite).Times(AtLeast(1));
  EXPECT_CALL(*arduinoMock, delay).Times(AtLeast(1));
  HardwareTest::beepCmd();
}

TEST_F(HardwareTestTest, test_setSingleCmd_fail1) {
  EXPECT_CALL(*serialCommandMock, next).WillOnce(Return(nullptr));
  HardwareTest::setSingleCmd();
}

TEST_F(HardwareTestTest, test_setSingleCmd_fail2) {
  EXPECT_CALL(*serialCommandMock, next).WillOnce(Return(twenty));
  HardwareTest::setSingleCmd();
}

TEST_F(HardwareTestTest, test_setSingleCmd_fail3) {
  EXPECT_CALL(*serialCommandMock, next)
      .WillOnce(Return(zero))
      .WillOnce(Return(nullptr));
  HardwareTest::setSingleCmd();
}

TEST_F(HardwareTestTest, test_setSingleCmd_fail4) {
  EXPECT_CALL(*serialCommandMock, next)
      .WillOnce(Return(zero))
      .WillOnce(Return(two));
  HardwareTest::setSingleCmd();
}

TEST_F(HardwareTestTest, test_setSingleCmd_success) {
  EXPECT_CALL(*serialCommandMock, next).WillRepeatedly(Return(zero));
  EXPECT_CALL(*knitterMock, setSolenoid);
  HardwareTest::setSingleCmd();
}

TEST_F(HardwareTestTest, test_setAllCmd_fail1) {
  EXPECT_CALL(*serialCommandMock, next).WillOnce(Return(nullptr));
  HardwareTest::setAllCmd();
}

TEST_F(HardwareTestTest, test_setAllCmd_fail2) {
  EXPECT_CALL(*serialCommandMock, next)
      .WillOnce(Return(zero))
      .WillOnce(Return(nullptr));
  HardwareTest::setAllCmd();
}

TEST_F(HardwareTestTest, test_setAllCmd_success) {
  EXPECT_CALL(*serialCommandMock, next).WillRepeatedly(Return(zero));
  EXPECT_CALL(*knitterMock, setSolenoids);
  HardwareTest::setAllCmd();
}

TEST_F(HardwareTestTest, test_readEOLsensorsCmd) {
  HardwareTest::readEOLsensorsCmd();
}

TEST_F(HardwareTestTest, test_readEncodersCmd) {
  // low
  EXPECT_CALL(*arduinoMock, digitalRead).WillRepeatedly(Return(0));
  HardwareTest::readEncodersCmd();

  // high
  EXPECT_CALL(*arduinoMock, digitalRead).WillRepeatedly(Return(1));
  HardwareTest::readEncodersCmd();
}

TEST_F(HardwareTestTest, test_autoReadCmd) {
  HardwareTest::autoReadCmd();
}

TEST_F(HardwareTestTest, test_autoTestCmd) {
  HardwareTest::autoTestCmd();
}

TEST_F(HardwareTestTest, test_stopCmd) {
  HardwareTest::stopCmd();
}

TEST_F(HardwareTestTest, test_quitCmd) {
  EXPECT_CALL(*knitterMock, setQuitFlag);
  HardwareTest::quitCmd();
}

TEST_F(HardwareTestTest, test_unrecognizedCmd) {
  const char buffer[1] = {1};
  HardwareTest::unrecognizedCmd(buffer);
}

TEST_F(HardwareTestTest, test_loop_default) {
  HardwareTest::m_lastTime = 0;
  EXPECT_CALL(*arduinoMock, millis).WillOnce(Return(499));
  HardwareTest::loop();
}

TEST_F(HardwareTestTest, test_loop_autoRead) {
  HardwareTest::m_lastTime = 0;
  EXPECT_CALL(*arduinoMock, millis).WillOnce(Return(500));
  HardwareTest::m_timerEventOdd = true;
  HardwareTest::m_autoReadOn = true;
  EXPECT_CALL(*arduinoMock, analogRead).Times(2);
  EXPECT_CALL(*arduinoMock, digitalRead).Times(3);
  HardwareTest::loop();
}

TEST_F(HardwareTestTest, test_loop_autoTestEven) {
  HardwareTest::m_lastTime = 0;
  EXPECT_CALL(*arduinoMock, millis).WillOnce(Return(500));
  HardwareTest::m_timerEventOdd = false;
  HardwareTest::m_autoTestOn = true;
  EXPECT_CALL(*arduinoMock, digitalWrite).Times(2);
  EXPECT_CALL(*knitterMock, setSolenoids);
  HardwareTest::loop();
}

TEST_F(HardwareTestTest, test_loop_autoTestOdd) {
  HardwareTest::m_lastTime = 0;
  EXPECT_CALL(*arduinoMock, millis).WillOnce(Return(500));
  HardwareTest::m_timerEventOdd = true;
  HardwareTest::m_autoTestOn = true;
  EXPECT_CALL(*arduinoMock, digitalWrite).Times(2);
  EXPECT_CALL(*knitterMock, setSolenoids);
  HardwareTest::loop();
}
