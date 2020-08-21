/*!`
 * \file test_tester.cpp
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
#include <com.h>
#include <tester.h>

#include <knitter_mock.h>

using ::testing::An;
using ::testing::AtLeast;
using ::testing::Mock;
using ::testing::Return;

extern Tester *tester;
extern KnitterMock *knitter;

// static char zero[2] = {'0', 0};
// static char two[2] = {'2', 0};
// static char g[2] = {'g', 0};
// static char fAdE[5] = {'f', 'A', 'd', 'E', 0};
// static char sixteen[3] = {'1', '6', 0};

class TesterTest : public ::testing::Test {
protected:
  void SetUp() override {
    arduinoMock = arduinoMockInstance();
    serialMock = serialMockInstance();
    // serialCommandMock = serialCommandMockInstance();

    // pointer to global instance
    knitterMock = knitter;

    // The global instance does not get destroyed at the end of each test.
    // Ordinarily the mock instance would be local and such behaviour would
    // cause a memory leak. We must notify the test that this is not the case.
    Mock::AllowLeak(knitterMock);
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
};

TEST_F(TesterTest, test_setUp) {
  EXPECT_CALL(*arduinoMock, millis);
  tester->setUp();
  ASSERT_FALSE(tester->m_autoReadOn);
  ASSERT_FALSE(tester->m_autoTestOn);
  ASSERT_FALSE(tester->m_timerEventOdd);
  ASSERT_FALSE(tester->getQuitFlag());
}

TEST_F(TesterTest, test_helpCmd) {
  // EXPECT_CALL(*knitterMock, sendMsg(test_msgid, An<const char *>()))
  //    .Times(AtLeast(1));
  tester->helpCmd();
}

TEST_F(TesterTest, test_sendCmd) {
  // EXPECT_CALL(*knitterMock, sendMsg(test_msgid, An<const char *>()))
  //    .Times(AtLeast(1));
  // EXPECT_CALL(*knitterMock, send);
  tester->sendCmd();
}

TEST_F(TesterTest, test_beepCmd) {
  // EXPECT_CALL(*knitterMock, sendMsg(test_msgid, An<const char *>()))
  //    .Times(AtLeast(1));
  EXPECT_CALL(*arduinoMock, analogWrite).Times(AtLeast(1));
  EXPECT_CALL(*arduinoMock, delay).Times(AtLeast(1));
  tester->beepCmd();
}

TEST_F(TesterTest, test_setSingleCmd_fail1) {
  // EXPECT_CALL(*knitterMock, sendMsg(test_msgid, An<const char *>()))
  //    .Times(AtLeast(1));
  // EXPECT_CALL(*serialCommandMock, next).WillOnce(Return(nullptr));
  EXPECT_CALL(*knitterMock, setSolenoids).Times(0);
  tester->setSingleCmd();
}

TEST_F(TesterTest, test_setSingleCmd_fail2) {
  // EXPECT_CALL(*knitterMock, sendMsg(test_msgid, An<const char *>()))
  //    .Times(AtLeast(1));
  // EXPECT_CALL(*serialCommandMock, next).WillOnce(Return(sixteen));
  EXPECT_CALL(*knitterMock, setSolenoids).Times(0);
  tester->setSingleCmd();
}

TEST_F(TesterTest, test_setSingleCmd_fail3) {
  // EXPECT_CALL(*knitterMock, sendMsg(test_msgid, An<const char *>()))
  //    .Times(AtLeast(1));
  // EXPECT_CALL(*serialCommandMock, next)
  //    .WillOnce(Return(zero))
  //    .WillOnce(Return(nullptr));
  EXPECT_CALL(*knitterMock, setSolenoids).Times(0);
  tester->setSingleCmd();
}

TEST_F(TesterTest, test_setSingleCmd_fail4) {
  // EXPECT_CALL(*knitterMock, sendMsg(test_msgid, An<const char *>()))
  //    .Times(AtLeast(1));
  // EXPECT_CALL(*serialCommandMock, next)
  //    .WillOnce(Return(zero))
  //    .WillOnce(Return(two));
  EXPECT_CALL(*knitterMock, setSolenoids).Times(0);
  tester->setSingleCmd();
}

TEST_F(TesterTest, test_setSingleCmd_success) {
  // EXPECT_CALL(*knitterMock, sendMsg(test_msgid, An<const char *>()))
  //    .Times(AtLeast(1));
  // EXPECT_CALL(*serialCommandMock, next).WillRepeatedly(Return(zero));
  EXPECT_CALL(*knitterMock, setSolenoids);
  tester->setSingleCmd();
}

TEST_F(TesterTest, test_setAllCmd_fail1) {
  // EXPECT_CALL(*knitterMock, sendMsg(test_msgid, An<const char *>()))
  //    .Times(AtLeast(1));
  // EXPECT_CALL(*serialCommandMock, next).WillOnce(Return(nullptr));
  EXPECT_CALL(*knitterMock, setSolenoids).Times(0);
  tester->setAllCmd();
}

TEST_F(TesterTest, test_setAllCmd_fail2) {
  // EXPECT_CALL(*knitterMock, sendMsg(test_msgid, An<const char *>()))
  //    .Times(AtLeast(1));
  // EXPECT_CALL(*serialCommandMock, next).WillOnce(Return(g));
  EXPECT_CALL(*knitterMock, setSolenoids).Times(0);
  tester->setAllCmd();
}

TEST_F(TesterTest, test_setAllCmd_success) {
  // EXPECT_CALL(*knitterMock, sendMsg(test_msgid, An<const char *>()))
  //    .Times(AtLeast(1));
  // EXPECT_CALL(*serialCommandMock, next).WillOnce(Return(fAdE));
  EXPECT_CALL(*knitterMock, setSolenoids);
  tester->setAllCmd();
}

TEST_F(TesterTest, test_readEOLsensorsCmd) {
  // EXPECT_CALL(*knitterMock, sendMsg(test_msgid, An<const char *>()))
  //    .Times(AtLeast(1));
  tester->readEOLsensorsCmd();
}

TEST_F(TesterTest, test_readEncodersCmd_low) {
  // EXPECT_CALL(*knitterMock, sendMsg(test_msgid, An<const char *>()))
  //    .Times(AtLeast(1));
  EXPECT_CALL(*arduinoMock, digitalRead).WillRepeatedly(Return(LOW));
  tester->readEncodersCmd();
}

TEST_F(TesterTest, test_readEncodersCmd_high) {
  // EXPECT_CALL(*knitterMock, sendMsg(test_msgid, An<const char *>()))
  //    .Times(AtLeast(1));
  EXPECT_CALL(*arduinoMock, digitalRead).WillRepeatedly(Return(HIGH));
  tester->readEncodersCmd();
}

TEST_F(TesterTest, test_autoReadCmd) {
  // EXPECT_CALL(*knitterMock, sendMsg(test_msgid, An<const char *>()))
  //    .Times(AtLeast(1));
  tester->autoReadCmd();
}

TEST_F(TesterTest, test_autoTestCmd) {
  // EXPECT_CALL(*knitterMock, sendMsg(test_msgid, An<const char *>()))
  //    .Times(AtLeast(1));
  tester->autoTestCmd();
}

TEST_F(TesterTest, test_stopCmd) {
  tester->m_autoReadOn = true;
  tester->m_autoTestOn = true;
  tester->stopCmd();
  ASSERT_FALSE(tester->m_autoReadOn);
  ASSERT_FALSE(tester->m_autoTestOn);
}

TEST_F(TesterTest, test_quitCmd) {
  tester->quitCmd();
  ASSERT_TRUE(tester->m_quit);
}

TEST_F(TesterTest, test_unrecognizedCmd) {
  // EXPECT_CALL(*knitterMock, sendMsg(test_msgid, An<const char *>()))
  //    .Times(AtLeast(1));
  const char buffer[1] = {1};
  tester->unrecognizedCmd(buffer);
}

TEST_F(TesterTest, test_loop_default) {
  tester->m_lastTime = 0;
  EXPECT_CALL(*arduinoMock, millis).WillOnce(Return(499));
  tester->loop();
}

TEST_F(TesterTest, test_loop_null) {
  tester->m_lastTime = 0;
  EXPECT_CALL(*arduinoMock, millis).WillOnce(Return(500));
  tester->m_autoReadOn = false;
  tester->m_autoTestOn = false;
  tester->loop();
}

TEST_F(TesterTest, test_loop_autoTestEven) {
  tester->m_lastTime = 0;
  EXPECT_CALL(*arduinoMock, millis).WillOnce(Return(500));
  tester->m_timerEventOdd = false;
  tester->m_autoReadOn = true;
  tester->m_autoTestOn = true;
  EXPECT_CALL(*arduinoMock, digitalRead).Times(0);
  EXPECT_CALL(*arduinoMock, digitalWrite).Times(2);
  EXPECT_CALL(*knitterMock, setSolenoids);
  tester->loop();
}

TEST_F(TesterTest, test_loop_autoTestOdd) {
  tester->m_lastTime = 0;
  EXPECT_CALL(*arduinoMock, millis).WillOnce(Return(500));
  tester->m_timerEventOdd = true;
  tester->m_autoReadOn = true;
  tester->m_autoTestOn = true;
  EXPECT_CALL(*arduinoMock, digitalRead).Times(3);
  EXPECT_CALL(*arduinoMock, digitalWrite).Times(2);
  EXPECT_CALL(*knitterMock, setSolenoids);
  tester->loop();
}

/*
TEST_F(TesterTest, test_scanHex) {
  uint16_t result;
  ASSERT_FALSE(tester->scanHex(zero, 0, &result));
  ASSERT_FALSE(tester->scanHex(g, 4, &result));
  ASSERT_FALSE(tester->scanHex(g + 1, 1, &result));
  ASSERT_TRUE(tester->scanHex(zero, 1, &result));
  ASSERT_TRUE(result == 0);
  ASSERT_TRUE(tester->scanHex(zero, 4, &result));
  ASSERT_TRUE(result == 0);
  ASSERT_TRUE(tester->scanHex(a, 1, &result));
  ASSERT_TRUE(result == 0xa);
  ASSERT_TRUE(tester->scanHex(A, 4, &result));
  ASSERT_TRUE(result == 0xA);
  ASSERT_TRUE(tester->scanHex(fAdE, 4, &result));
  ASSERT_TRUE(result == 0xfAdE);
}
*/
