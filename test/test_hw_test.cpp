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

//#include <SerialCommand_mock.h>
#include <global_hw_test.h>
#include <knitter_mock.h>

using ::testing::An;
using ::testing::AtLeast;
using ::testing::Return;

// static char zero[2] = {'0', 0};
// static char two[2] = {'2', 0};
// static char g[2] = {'g', 0};
// static char fAdE[5] = {'f', 'A', 'd', 'E', 0};
// static char sixteen[3] = {'1', '6', 0};

// initialize static member
HardwareTestInterface *GlobalHardwareTest::m_instance = new HardwareTest();

class HardwareTestTest : public ::testing::Test {
protected:
  void SetUp() override {
    arduinoMock = arduinoMockInstance();
    serialMock = serialMockInstance();
    // serialCommandMock = serialCommandMockInstance();
    knitterMock = knitterMockInstance();
    h = new HardwareTest();
  }

  void TearDown() override {
    releaseArduinoMock();
    releaseSerialMock();
    // releaseSerialCommandMock();
    releaseKnitterMock();
  }

  ArduinoMock *arduinoMock;
  SerialMock *serialMock;
  // SerialCommandMock *serialCommandMock;
  KnitterMock *knitterMock;
  HardwareTest *h;
};

TEST_F(HardwareTestTest, test_setUp) {
  EXPECT_CALL(*knitterMock, setQuitFlag);
  EXPECT_CALL(*arduinoMock, millis);
  h->setUp();
  ASSERT_FALSE(h->m_autoReadOn);
  ASSERT_FALSE(h->m_autoTestOn);
  ASSERT_FALSE(h->m_timerEventOdd);
}

TEST_F(HardwareTestTest, test_helpCmd) {
  EXPECT_CALL(*knitterMock, sendMsg(test_msgid, An<const char *>()))
      .Times(AtLeast(1));
  h->helpCmd();
}

TEST_F(HardwareTestTest, test_sendCmd) {
  EXPECT_CALL(*knitterMock, sendMsg(test_msgid, An<const char *>()))
      .Times(AtLeast(1));
  EXPECT_CALL(*knitterMock, send);
  h->sendCmd();
}

TEST_F(HardwareTestTest, test_beepCmd) {
  EXPECT_CALL(*knitterMock, sendMsg(test_msgid, An<const char *>()))
      .Times(AtLeast(1));
  EXPECT_CALL(*arduinoMock, analogWrite).Times(AtLeast(1));
  EXPECT_CALL(*arduinoMock, delay).Times(AtLeast(1));
  h->beepCmd();
}

TEST_F(HardwareTestTest, test_setSingleCmd_fail1) {
  EXPECT_CALL(*knitterMock, sendMsg(test_msgid, An<const char *>()))
      .Times(AtLeast(1));
  // EXPECT_CALL(*serialCommandMock, next).WillOnce(Return(nullptr));
  // EXPECT_CALL(*knitterMock, setSolenoid).Times(0);
  h->setSingleCmd();
}

TEST_F(HardwareTestTest, test_setSingleCmd_fail2) {
  EXPECT_CALL(*knitterMock, sendMsg(test_msgid, An<const char *>()))
      .Times(AtLeast(1));
  // EXPECT_CALL(*serialCommandMock, next).WillOnce(Return(sixteen));
  // EXPECT_CALL(*knitterMock, setSolenoid).Times(0);
  h->setSingleCmd();
}

TEST_F(HardwareTestTest, test_setSingleCmd_fail3) {
  EXPECT_CALL(*knitterMock, sendMsg(test_msgid, An<const char *>()))
      .Times(AtLeast(1));
  // EXPECT_CALL(*serialCommandMock, next)
  //    .WillOnce(Return(zero))
  //    .WillOnce(Return(nullptr));
  // EXPECT_CALL(*knitterMock, setSolenoid).Times(0);
  h->setSingleCmd();
}

TEST_F(HardwareTestTest, test_setSingleCmd_fail4) {
  EXPECT_CALL(*knitterMock, sendMsg(test_msgid, An<const char *>()))
      .Times(AtLeast(1));
  // EXPECT_CALL(*serialCommandMock, next)
  //    .WillOnce(Return(zero))
  //    .WillOnce(Return(two));
  // EXPECT_CALL(*knitterMock, setSolenoid).Times(0);
  h->setSingleCmd();
}

TEST_F(HardwareTestTest, test_setSingleCmd_success) {
  EXPECT_CALL(*knitterMock, sendMsg(test_msgid, An<const char *>()))
      .Times(AtLeast(1));
  // EXPECT_CALL(*serialCommandMock, next).WillRepeatedly(Return(zero));
  // EXPECT_CALL(*knitterMock, setSolenoid);
  h->setSingleCmd();
}

TEST_F(HardwareTestTest, test_setAllCmd_fail1) {
  EXPECT_CALL(*knitterMock, sendMsg(test_msgid, An<const char *>()))
      .Times(AtLeast(1));
  // EXPECT_CALL(*serialCommandMock, next).WillOnce(Return(nullptr));
  // EXPECT_CALL(*knitterMock, setSolenoids).Times(0);
  h->setAllCmd();
}

TEST_F(HardwareTestTest, test_setAllCmd_fail2) {
  EXPECT_CALL(*knitterMock, sendMsg(test_msgid, An<const char *>()))
      .Times(AtLeast(1));
  // EXPECT_CALL(*serialCommandMock, next).WillOnce(Return(g));
  // EXPECT_CALL(*knitterMock, setSolenoids).Times(0);
  h->setAllCmd();
}

TEST_F(HardwareTestTest, test_setAllCmd_success) {
  EXPECT_CALL(*knitterMock, sendMsg(test_msgid, An<const char *>()))
      .Times(AtLeast(1));
  // EXPECT_CALL(*serialCommandMock, next).WillOnce(Return(fAdE));
  // EXPECT_CALL(*knitterMock, setSolenoids);
  h->setAllCmd();
}

TEST_F(HardwareTestTest, test_readEOLsensorsCmd) {
  EXPECT_CALL(*knitterMock, sendMsg(test_msgid, An<const char *>()))
      .Times(AtLeast(1));
  h->readEOLsensorsCmd();
}

TEST_F(HardwareTestTest, test_readEncodersCmd_low) {
  EXPECT_CALL(*knitterMock, sendMsg(test_msgid, An<const char *>()))
      .Times(AtLeast(1));
  EXPECT_CALL(*arduinoMock, digitalRead).WillRepeatedly(Return(LOW));
  h->readEncodersCmd();
}

TEST_F(HardwareTestTest, test_readEncodersCmd_high) {
  EXPECT_CALL(*knitterMock, sendMsg(test_msgid, An<const char *>()))
      .Times(AtLeast(1));
  EXPECT_CALL(*arduinoMock, digitalRead).WillRepeatedly(Return(HIGH));
  h->readEncodersCmd();
}

TEST_F(HardwareTestTest, test_autoReadCmd) {
  EXPECT_CALL(*knitterMock, sendMsg(test_msgid, An<const char *>()))
      .Times(AtLeast(1));
  h->autoReadCmd();
}

TEST_F(HardwareTestTest, test_autoTestCmd) {
  EXPECT_CALL(*knitterMock, sendMsg(test_msgid, An<const char *>()))
      .Times(AtLeast(1));
  h->autoTestCmd();
}

TEST_F(HardwareTestTest, test_stopCmd) {
  h->m_autoReadOn = true;
  h->m_autoTestOn = true;
  h->stopCmd();
  ASSERT_FALSE(h->m_autoReadOn);
  ASSERT_FALSE(h->m_autoTestOn);
}

TEST_F(HardwareTestTest, test_quitCmd) {
  EXPECT_CALL(*knitterMock, setQuitFlag);
  h->quitCmd();
}

TEST_F(HardwareTestTest, test_unrecognizedCmd) {
  EXPECT_CALL(*knitterMock, sendMsg(test_msgid, An<const char *>()))
      .Times(AtLeast(1));
  const char buffer[1] = {1};
  h->unrecognizedCmd(buffer);
}

TEST_F(HardwareTestTest, test_loop_default) {
  h->m_lastTime = 0;
  EXPECT_CALL(*arduinoMock, millis).WillOnce(Return(499));
  h->loop();
}

TEST_F(HardwareTestTest, test_loop_null) {
  h->m_lastTime = 0;
  EXPECT_CALL(*arduinoMock, millis).WillOnce(Return(500));
  h->m_autoReadOn = false;
  h->m_autoTestOn = false;
  h->loop();
}

TEST_F(HardwareTestTest, test_loop_autoTestEven) {
  h->m_lastTime = 0;
  EXPECT_CALL(*arduinoMock, millis).WillOnce(Return(500));
  h->m_timerEventOdd = false;
  h->m_autoReadOn = true;
  h->m_autoTestOn = true;
  EXPECT_CALL(*arduinoMock, digitalRead).Times(0);
  EXPECT_CALL(*arduinoMock, digitalWrite).Times(2);
  EXPECT_CALL(*knitterMock, setSolenoids);
  h->loop();
}

TEST_F(HardwareTestTest, test_loop_autoTestOdd) {
  h->m_lastTime = 0;
  EXPECT_CALL(*arduinoMock, millis).WillOnce(Return(500));
  h->m_timerEventOdd = true;
  h->m_autoReadOn = true;
  h->m_autoTestOn = true;
  EXPECT_CALL(*arduinoMock, digitalRead).Times(3);
  EXPECT_CALL(*arduinoMock, digitalWrite).Times(2);
  EXPECT_CALL(*knitterMock, setSolenoids);
  h->loop();
}

/*
TEST_F(HardwareTestTest, test_scanHex) {
  uint16_t result;
  ASSERT_FALSE(h->scanHex(zero, 0, &result));
  ASSERT_FALSE(h->scanHex(g, 4, &result));
  ASSERT_FALSE(h->scanHex(g + 1, 1, &result));
  ASSERT_TRUE(h->scanHex(zero, 1, &result));
  ASSERT_TRUE(result == 0);
  ASSERT_TRUE(h->scanHex(zero, 4, &result));
  ASSERT_TRUE(result == 0);
  ASSERT_TRUE(h->scanHex(a, 1, &result));
  ASSERT_TRUE(result == 0xa);
  ASSERT_TRUE(h->scanHex(A, 4, &result));
  ASSERT_TRUE(result == 0xA);
  ASSERT_TRUE(h->scanHex(fAdE, 4, &result));
  ASSERT_TRUE(result == 0xfAdE);
}
*/
