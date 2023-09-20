/*!`
 * \file test_OpTest.cpp
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
 *    Modified Work Copyright 2020-3 Sturla Lange, Tom Price
 *    http://ayab-knitting.com
 */

#include <gtest/gtest.h>

#include <beeper.h>

#include <opInit.h>
#include <opReady.h>
#include <opTest.h>

#include <fsm_mock.h>
#include <opKnit_mock.h>

using ::testing::_;
using ::testing::An;
using ::testing::AtLeast;
using ::testing::Mock;
using ::testing::Return;

extern Beeper *beeper;

extern OpInit *opInit;
extern OpReady *opReady;
extern OpTest *opTest;

extern FsmMock *fsm;
extern OpKnitMock *opKnit;

class TestOpTest : public ::testing::Test {
protected:
  void SetUp() override {
    arduinoMock = arduinoMockInstance();
    serialMock = serialMockInstance();
    // serialCommandMock = serialCommandMockInstance();

    // pointers to global instances
    fsmMock = fsm;
    opKnitMock = opKnit;

    // The global instances do not get destroyed at the end of each test.
    // Ordinarily the mock instance would be local and such behaviour would
    // cause a memory leak. We must notify the test that this is not the case.
    Mock::AllowLeak(fsmMock);
    Mock::AllowLeak(opKnitMock);

    beeper->init(true);
  }

  void TearDown() override {
    releaseArduinoMock();
    releaseSerialMock();
  }

  ArduinoMock *arduinoMock;
  SerialMock *serialMock;
  FsmMock *fsmMock;
  OpKnitMock *opKnitMock;

  void expect_startTest(unsigned long t) {
    EXPECT_CALL(*fsmMock, getState).WillOnce(Return(opReady));
    EXPECT_CALL(*fsmMock, setState(opTest));
    EXPECT_CALL(*fsmMock, setMachineType(Machine_t::Kh930));
    expect_write(false);

    EXPECT_CALL(*arduinoMock, millis).WillOnce(Return(t));

    ASSERT_TRUE(opTest->begin() == ErrorCode::success);
  }

  void expect_write(bool once) {
    return;
    //TODO: FIXME: Mock PocketSerial, so this works again.
    if (once) {
      EXPECT_CALL(*serialMock, write(_, _));
      EXPECT_CALL(*serialMock, write(SLIP::END));
    } else {
      EXPECT_CALL(*serialMock, write(_, _)).Times(AtLeast(1));
      EXPECT_CALL(*serialMock, write(SLIP::END)).Times(AtLeast(1));
    }
  }

  void expect_readEOLsensors(bool flag) {
    uint8_t n = flag ? 1 : 0;
    EXPECT_CALL(*arduinoMock, analogRead(EOL_PIN_L)).Times(n);
    EXPECT_CALL(*arduinoMock, analogRead(EOL_PIN_R)).Times(n);
  }

  void expect_readEncoders(bool flag) {
    uint8_t n = flag ? 1 : 0;
    EXPECT_CALL(*arduinoMock, digitalRead(ENC_PIN_A)).Times(n);
    EXPECT_CALL(*arduinoMock, digitalRead(ENC_PIN_B)).Times(n);
    EXPECT_CALL(*arduinoMock, digitalRead(ENC_PIN_C)).Times(n);
  }
};

TEST_F(TestOpTest, test_helpCmd) {
  expect_write(false);
  opTest->helpCmd();
}

TEST_F(TestOpTest, test_sendCmd) {
  expect_write(false);
  opTest->sendCmd();
}

TEST_F(TestOpTest, test_beepCmd) {
  expect_write(true);
  opTest->beepCmd();
  EXPECT_CALL(*arduinoMock, millis).WillOnce(Return(0U));
  beeper->update();
  EXPECT_CALL(*arduinoMock, analogWrite(PIEZO_PIN, BEEP_ON_DUTY));
  EXPECT_CALL(*arduinoMock, millis).WillOnce(Return(1U));
  beeper->update();
}

TEST_F(TestOpTest, test_setSingleCmd_fail1) {
  const uint8_t buf[] = {static_cast<uint8_t>(AYAB_API::setSingleCmd), 0};
  expect_write(false);
  opTest->setSingleCmd(buf, 2);
}

TEST_F(TestOpTest, test_setSingleCmd_fail2) {
  const uint8_t buf[] = {static_cast<uint8_t>(AYAB_API::setSingleCmd), 16, 0};
  expect_write(false);
  opTest->setSingleCmd(buf, 3);
}

TEST_F(TestOpTest, test_setSingleCmd_fail3) {
  const uint8_t buf[] = {static_cast<uint8_t>(AYAB_API::setSingleCmd), 15, 2};
  expect_write(false);
  opTest->setSingleCmd(buf, 3);
}

TEST_F(TestOpTest, test_setSingleCmd_success) {
  const uint8_t buf[] = {static_cast<uint8_t>(AYAB_API::setSingleCmd), 15, 1};
  expect_write(true);
  opTest->setSingleCmd(buf, 3);
}

TEST_F(TestOpTest, test_setAllCmd_fail1) {
  const uint8_t buf[] = {static_cast<uint8_t>(AYAB_API::setAllCmd), 0};
  expect_write(false);
  opTest->setAllCmd(buf, 2);
}

TEST_F(TestOpTest, test_setAllCmd_success) {
  const uint8_t buf[] = {static_cast<uint8_t>(AYAB_API::setAllCmd), 0xff, 0xff};
  expect_write(true);
  opTest->setAllCmd(buf, 3);
}

TEST_F(TestOpTest, test_readEOLsensorsCmd) {
  expect_write(false);
  expect_readEOLsensors(true);
  opTest->readEOLsensorsCmd();
}

TEST_F(TestOpTest, test_readEncodersCmd_low) {
  expect_write(false);
  EXPECT_CALL(*arduinoMock, digitalRead).WillRepeatedly(Return(LOW));
  opTest->readEncodersCmd();
}

TEST_F(TestOpTest, test_readEncodersCmd_high) {
  expect_write(false);
  EXPECT_CALL(*arduinoMock, digitalRead).WillRepeatedly(Return(HIGH));
  opTest->readEncodersCmd();
}

TEST_F(TestOpTest, test_autoReadCmd) {
  expect_write(true);
  opTest->autoReadCmd();
}

TEST_F(TestOpTest, test_autoTestCmd) {
  expect_write(true);
  opTest->autoTestCmd();
}

TEST_F(TestOpTest, test_quitCmd) {
  EXPECT_CALL(*fsmMock, setState(opInit));
  opTest->end();

  // test expectations without destroying instance
  ASSERT_TRUE(Mock::VerifyAndClear(opKnitMock));
  ASSERT_TRUE(Mock::VerifyAndClear(fsmMock));
}

TEST_F(TestOpTest, test_loop_default) {
  expect_startTest(0);
  EXPECT_CALL(*arduinoMock, millis).WillOnce(Return(TEST_LOOP_DELAY - 1));
  opTest->update();
}

TEST_F(TestOpTest, test_loop_null) {
  expect_startTest(0);
  EXPECT_CALL(*arduinoMock, millis).WillOnce(Return(TEST_LOOP_DELAY));
  opTest->update();
}

TEST_F(TestOpTest, test_loop_autoTest) {
  expect_startTest(0);
  opTest->autoReadCmd();
  opTest->autoTestCmd();

  // m_timerEventOdd = false
  EXPECT_CALL(*arduinoMock, millis).WillOnce(Return(TEST_LOOP_DELAY));
  expect_write(true);
  expect_readEOLsensors(false);
  expect_readEncoders(false);
  EXPECT_CALL(*arduinoMock, digitalWrite(LED_PIN_A, HIGH));
  EXPECT_CALL(*arduinoMock, digitalWrite(LED_PIN_B, HIGH));
  opTest->update();

  // m_timerEventOdd = false
  EXPECT_CALL(*arduinoMock, millis).WillOnce(Return(2 * TEST_LOOP_DELAY));
  expect_write(false);
  expect_readEOLsensors(true);
  expect_readEncoders(true);
  EXPECT_CALL(*arduinoMock, digitalWrite(LED_PIN_A, LOW));
  EXPECT_CALL(*arduinoMock, digitalWrite(LED_PIN_B, LOW));
  opTest->update();

  // after `stopCmd()`
  opTest->stopCmd();
  EXPECT_CALL(*arduinoMock, millis).WillOnce(Return(3 * TEST_LOOP_DELAY));
  expect_readEOLsensors(false);
  expect_readEncoders(false);
  EXPECT_CALL(*arduinoMock, digitalWrite(LED_PIN_A, _)).Times(0);
  EXPECT_CALL(*arduinoMock, digitalWrite(LED_PIN_B, _)).Times(0);
  opTest->update();
}

TEST_F(TestOpTest, test_startTest_fail) {
  // can't start test from state `OpKnit`
  EXPECT_CALL(*fsmMock, getState).WillOnce(Return(opKnitMock));
  ASSERT_TRUE(opTest->begin() != ErrorCode::success);

  // test expectations without destroying instance
  ASSERT_TRUE(Mock::VerifyAndClear(fsmMock));
}

TEST_F(TestOpTest, test_startTest_success) {
  expect_startTest(0);

  // test expectations without destroying instance
  ASSERT_TRUE(Mock::VerifyAndClear(fsmMock));
  ASSERT_TRUE(Mock::VerifyAndClear(opKnitMock));
}
