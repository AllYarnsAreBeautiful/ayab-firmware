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

#include <controller_mock.h>
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

extern OpKnitMock *opKnit;
extern ControllerMock *controller;

class OpTestTest : public ::testing::Test {
protected:
  void SetUp() override {
    arduinoMock = arduinoMockInstance();
    serialMock = serialMockInstance();
    // serialCommandMock = serialCommandMockInstance();

    // pointers to global instances
    controllerMock = controller;
    opKnitMock = opKnit;

    // The global instances do not get destroyed at the end of each test.
    // Ordinarily the mock instance would be local and such behaviour would
    // cause a memory leak. We must notify the test that this is not the case.
    Mock::AllowLeak(controllerMock);
    Mock::AllowLeak(opKnitMock);

    beeper->init(true);
  }

  void TearDown() override {
    releaseArduinoMock();
    releaseSerialMock();
  }

  ArduinoMock *arduinoMock;
  SerialMock *serialMock;
  ControllerMock *controllerMock;
  OpKnitMock *opKnitMock;

  void expect_startTest(uint32_t t) {
    expect_write(false);
    EXPECT_CALL(*arduinoMock, millis).WillOnce(Return(t));
    opTest->begin();
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

TEST_F(OpTestTest, test_state) {
  ASSERT_EQ(opTest->state(), OpState_t::Test);
}

TEST_F(OpTestTest, test_init) {
  // no calls expected
  opTest->init();
}

TEST_F(OpTestTest, test_enabled) {
  const uint8_t stopCmd[] = {static_cast<uint8_t>(API_t::stopCmd)};
  const uint8_t autoReadCmd[] = {static_cast<uint8_t>(API_t::autoReadCmd)};
  const uint8_t autoTestCmd[] = {static_cast<uint8_t>(API_t::autoTestCmd)};
  opTest->com(stopCmd, 1);
  ASSERT_EQ(opTest->enabled(), false);
  opTest->com(autoReadCmd, 1);
  ASSERT_EQ(opTest->enabled(), true);
  opTest->com(autoTestCmd, 1);
  ASSERT_EQ(opTest->enabled(), true);
  opTest->com(stopCmd, 1);
  opTest->com(autoTestCmd, 1);
  ASSERT_EQ(opTest->enabled(), true);
}

TEST_F(OpTestTest, test_helpCmd) {
  expect_write(false);
  opTest->helpCmd();
}

TEST_F(OpTestTest, test_sendCmd) {
  expect_write(false);
  opTest->sendCmd();
}

TEST_F(OpTestTest, test_beepCmd) {
  expect_write(true);
  opTest->beepCmd();
  EXPECT_CALL(*arduinoMock, millis).WillOnce(Return(0U));
  beeper->update();
  EXPECT_CALL(*arduinoMock, analogWrite(PIEZO_PIN, BEEP_ON_DUTY));
  EXPECT_CALL(*arduinoMock, millis).WillOnce(Return(1U));
  beeper->update();
}

TEST_F(OpTestTest, test_setSingleCmd_fail1) {
  const uint8_t buf[] = {static_cast<uint8_t>(API_t::setSingleCmd), 0};
  expect_write(false);
  opTest->setSingleCmd(buf, 2);
}

TEST_F(OpTestTest, test_setSingleCmd_fail2) {
  const uint8_t buf[] = {static_cast<uint8_t>(API_t::setSingleCmd), 16, 0};
  expect_write(false);
  opTest->setSingleCmd(buf, 3);
}

TEST_F(OpTestTest, test_setSingleCmd_fail3) {
  const uint8_t buf[] = {static_cast<uint8_t>(API_t::setSingleCmd), 15, 2};
  expect_write(false);
  opTest->setSingleCmd(buf, 3);
}

TEST_F(OpTestTest, test_setSingleCmd_success) {
  const uint8_t buf[] = {static_cast<uint8_t>(API_t::setSingleCmd), 15, 1};
  expect_write(true);
  opTest->setSingleCmd(buf, 3);
}

TEST_F(OpTestTest, test_setAllCmd_fail1) {
  const uint8_t buf[] = {static_cast<uint8_t>(API_t::setAllCmd), 0};
  expect_write(false);
  opTest->setAllCmd(buf, 2);
}

TEST_F(OpTestTest, test_setAllCmd_success) {
  const uint8_t buf[] = {static_cast<uint8_t>(API_t::setAllCmd), 0xFF, 0xFF};
  expect_write(true);
  opTest->setAllCmd(buf, 3);
}

TEST_F(OpTestTest, test_readEOLsensorsCmd) {
  expect_write(false);
  expect_readEOLsensors(true);
  opTest->readEOLsensorsCmd();
}

TEST_F(OpTestTest, test_readEncodersCmd_low) {
  expect_write(false);
  EXPECT_CALL(*arduinoMock, digitalRead).WillRepeatedly(Return(LOW));
  opTest->readEncodersCmd();
}

TEST_F(OpTestTest, test_readEncodersCmd_high) {
  expect_write(false);
  EXPECT_CALL(*arduinoMock, digitalRead).WillRepeatedly(Return(HIGH));
  opTest->readEncodersCmd();
}

TEST_F(OpTestTest, test_autoReadCmd) {
  const uint8_t buf[] = {static_cast<uint8_t>(API_t::autoReadCmd)};
  expect_write(true);
  opTest->com(buf, 1);
}

TEST_F(OpTestTest, test_autoTestCmd) {
  const uint8_t buf[] = {static_cast<uint8_t>(API_t::autoTestCmd)};
  expect_write(true);
  opTest->com(buf, 1);
}

TEST_F(OpTestTest, test_quitCmd) {
  const uint8_t buf[] = {static_cast<uint8_t>(API_t::quitCmd)};
  EXPECT_CALL(*opKnitMock, init);
  EXPECT_CALL(*controllerMock, setState(opInit));
  opTest->com(buf, 1);

  // test expectations without destroying instance
  ASSERT_TRUE(Mock::VerifyAndClear(opKnitMock));
  ASSERT_TRUE(Mock::VerifyAndClear(controllerMock));
}

TEST_F(OpTestTest, test_loop_null) {
  expect_startTest(0U);
  EXPECT_CALL(*arduinoMock, millis).Times(0);
  opTest->update();
}

TEST_F(OpTestTest, test_autoRead) {
  expect_startTest(0U);
  opTest->autoReadCmd();

  // nothing has happened yet
  EXPECT_CALL(*arduinoMock, millis).WillOnce(Return(TEST_LOOP_DELAY - 1));
  EXPECT_CALL(*opKnitMock, encodePosition);
  expect_write(false);
  expect_readEOLsensors(false);
  expect_readEncoders(false);
  opTest->update();

  // m_timerEventOdd = false
  EXPECT_CALL(*arduinoMock, millis).WillOnce(Return(TEST_LOOP_DELAY));
  EXPECT_CALL(*opKnitMock, encodePosition);
  expect_write(true);
  expect_readEOLsensors(false);
  expect_readEncoders(false);
  opTest->update();

  // m_timerEventOdd = false
  EXPECT_CALL(*arduinoMock, millis).WillOnce(Return(2 * TEST_LOOP_DELAY));
  EXPECT_CALL(*opKnitMock, encodePosition);
  expect_write(false);
  expect_readEOLsensors(true);
  expect_readEncoders(true);
  opTest->update();

  // after `stopCmd()`
  opTest->stopCmd();
  EXPECT_CALL(*arduinoMock, millis).Times(0);
  EXPECT_CALL(*opKnitMock, encodePosition).Times(0);
  expect_write(false);
  expect_readEOLsensors(false);
  expect_readEncoders(false);
  opTest->update();

  // test expectations without destroying instance
  ASSERT_TRUE(Mock::VerifyAndClear(opKnitMock));
}

TEST_F(OpTestTest, test_autoTest) {
  expect_startTest(0U);
  opTest->autoTestCmd();

  // nothing has happened yet
  EXPECT_CALL(*arduinoMock, millis).WillOnce(Return(TEST_LOOP_DELAY - 1));
  EXPECT_CALL(*opKnitMock, encodePosition);
  expect_write(false);
  EXPECT_CALL(*arduinoMock, digitalWrite(LED_PIN_A, HIGH)).Times(0);
  EXPECT_CALL(*arduinoMock, digitalWrite(LED_PIN_B, HIGH)).Times(0);
  opTest->update();

  // m_timerEventOdd = false
  EXPECT_CALL(*arduinoMock, millis).WillOnce(Return(TEST_LOOP_DELAY));
  EXPECT_CALL(*opKnitMock, encodePosition);
  expect_write(true);
  EXPECT_CALL(*arduinoMock, digitalWrite(LED_PIN_A, HIGH));
  EXPECT_CALL(*arduinoMock, digitalWrite(LED_PIN_B, HIGH));
  opTest->update();

  // m_timerEventOdd = false
  EXPECT_CALL(*arduinoMock, millis).WillOnce(Return(2 * TEST_LOOP_DELAY));
  EXPECT_CALL(*opKnitMock, encodePosition);
  expect_write(false);
  EXPECT_CALL(*arduinoMock, digitalWrite(LED_PIN_A, LOW));
  EXPECT_CALL(*arduinoMock, digitalWrite(LED_PIN_B, LOW));
  opTest->update();

  // after `stopCmd()`
  opTest->stopCmd();
  EXPECT_CALL(*arduinoMock, millis).Times(0);
  EXPECT_CALL(*opKnitMock, encodePosition).Times(0);
  expect_write(false);
  EXPECT_CALL(*arduinoMock, digitalWrite(LED_PIN_A, _)).Times(0);
  EXPECT_CALL(*arduinoMock, digitalWrite(LED_PIN_B, _)).Times(0);
  opTest->update();

  // test expectations without destroying instance
  ASSERT_TRUE(Mock::VerifyAndClear(opKnitMock));
}

TEST_F(OpTestTest, test_startTest_success) {
  expect_startTest(0U);
}

TEST_F(OpTestTest, test_unrecognized) {
  // no calls expected
  const uint8_t buffer[] = {0xFF};
  opTest->com(buffer, 1);
}

