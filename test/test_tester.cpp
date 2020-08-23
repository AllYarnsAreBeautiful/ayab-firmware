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

#include <com.h>
#include <tester.h>

#include <fsm_mock.h>
#include <knitter_mock.h>

using ::testing::An;
using ::testing::AtLeast;
using ::testing::Mock;
using ::testing::Return;

extern Tester *tester;

extern FsmMock *fsm;
extern KnitterMock *knitter;

class TesterTest : public ::testing::Test {
protected:
  void SetUp() override {
    arduinoMock = arduinoMockInstance();
    serialMock = serialMockInstance();
    // serialCommandMock = serialCommandMockInstance();

    // pointers to global instances
    fsmMock = fsm;
    knitterMock = knitter;

    // The global instances do not get destroyed at the end of each test.
    // Ordinarily the mock instance would be local and such behaviour would
    // cause a memory leak. We must notify the test that this is not the case.
    Mock::AllowLeak(fsmMock);
    Mock::AllowLeak(knitterMock);
  }

  void TearDown() override {
    releaseArduinoMock();
    releaseSerialMock();
  }

  ArduinoMock *arduinoMock;
  SerialMock *serialMock;
  FsmMock *fsmMock;
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
  tester->helpCmd();
}

TEST_F(TesterTest, test_sendCmd) {
  tester->sendCmd();
}

TEST_F(TesterTest, test_beepCmd) {
  EXPECT_CALL(*arduinoMock, analogWrite).Times(AtLeast(1));
  EXPECT_CALL(*arduinoMock, delay).Times(AtLeast(1));
  tester->beepCmd();
}

TEST_F(TesterTest, test_setSingleCmd_fail1) {
  const uint8_t buf[] = {setSingleCmd_msgid, 0};
  tester->setSingleCmd(buf, 2);
}

TEST_F(TesterTest, test_setSingleCmd_fail2) {
  const uint8_t buf[] = {setSingleCmd_msgid, 16, 0};
  tester->setSingleCmd(buf, 3);
}

TEST_F(TesterTest, test_setSingleCmd_fail3) {
  const uint8_t buf[] = {setSingleCmd_msgid, 15, 2};
  tester->setSingleCmd(buf, 3);
}

TEST_F(TesterTest, test_setSingleCmd_success) {
  const uint8_t buf[] = {setSingleCmd_msgid, 15, 1};
  tester->setSingleCmd(buf, 3);
}

TEST_F(TesterTest, test_setAllCmd_fail1) {
  const uint8_t buf[] = {setAllCmd_msgid, 0};
  tester->setAllCmd(buf, 2);
}

TEST_F(TesterTest, test_setAllCmd_success) {
  const uint8_t buf[] = {setAllCmd_msgid, 0xff, 0xff};
  tester->setAllCmd(buf, 3);
}

TEST_F(TesterTest, test_readEOLsensorsCmd) {
  EXPECT_CALL(*arduinoMock, analogRead(EOL_PIN_L));
  EXPECT_CALL(*arduinoMock, analogRead(EOL_PIN_R));
  tester->readEOLsensorsCmd();
}

TEST_F(TesterTest, test_readEncodersCmd_low) {
  EXPECT_CALL(*arduinoMock, digitalRead).WillRepeatedly(Return(LOW));
  tester->readEncodersCmd();
}

TEST_F(TesterTest, test_readEncodersCmd_high) {
  EXPECT_CALL(*arduinoMock, digitalRead).WillRepeatedly(Return(HIGH));
  tester->readEncodersCmd();
}

TEST_F(TesterTest, test_autoReadCmd) {
  tester->autoReadCmd();
}

TEST_F(TesterTest, test_autoTestCmd) {
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
  EXPECT_CALL(*knitterMock, setUpInterrupt);
  tester->quitCmd();
  ASSERT_TRUE(tester->m_quit);

  // test expectations without destroying instance
  ASSERT_TRUE(Mock::VerifyAndClear(knitterMock));
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
  tester->loop();
}

TEST_F(TesterTest, test_loop_autoTestOdd) {
  tester->m_lastTime = 0;
  EXPECT_CALL(*arduinoMock, millis).WillOnce(Return(500));
  tester->m_timerEventOdd = true;
  tester->m_autoReadOn = true;
  tester->m_autoTestOn = true;
  EXPECT_CALL(*arduinoMock, analogRead(EOL_PIN_L));
  EXPECT_CALL(*arduinoMock, analogRead(EOL_PIN_R));
  EXPECT_CALL(*arduinoMock, digitalRead).Times(3);
  EXPECT_CALL(*arduinoMock, digitalWrite).Times(2);
  tester->loop();
}

TEST_F(TesterTest, test_startTest_fail) {
  // can't start test from state `s_knit`
  EXPECT_CALL(*fsmMock, getState).WillOnce(Return(s_knit));
  ASSERT_EQ(tester->startTest(Kh910), false);

  // test expectations without destroying instance
  ASSERT_TRUE(Mock::VerifyAndClear(fsmMock));
}

TEST_F(TesterTest, test_startTest_success) {
  EXPECT_CALL(*fsmMock, getState).WillOnce(Return(s_ready));
  ASSERT_EQ(tester->startTest(Kh930), true);

  // test expectations without destroying instance
  ASSERT_TRUE(Mock::VerifyAndClear(fsmMock));
}
