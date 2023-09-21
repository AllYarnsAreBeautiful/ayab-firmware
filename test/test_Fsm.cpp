/*!`
 * \file test_fsm.cpp
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

#include <encoders.h>
#include <fsm.h>
#include <opKnit.h>

#include <beeper_mock.h>
#include <com_mock.h>
#include <encoders_mock.h>
#include <solenoids_mock.h>

#include <opIdle_mock.h>
#include <opInit_mock.h>
#include <opReady_mock.h>
#include <opTest_mock.h>
#include <opError_mock.h>

using ::testing::_;
using ::testing::AtLeast;
using ::testing::Mock;
using ::testing::Return;
using ::testing::Test;

extern Fsm *fsm;
extern OpKnit *opKnit;

extern BeeperMock *beeper;
extern ComMock *com;
extern EncodersMock *encoders;
extern SolenoidsMock *solenoids;

extern OpIdleMock  *opIdle;
extern OpInitMock  *opInit;
extern OpReadyMock *opReady;
extern OpTestMock  *opTest;
extern OpErrorMock *opError;

// Defaults for position
const uint8_t positionPassedLeft = (END_LEFT_PLUS_OFFSET[static_cast<uint8_t>(Machine_t::Kh910)] + GARTER_SLOP) + 1;
const uint8_t positionPassedRight = (END_RIGHT_MINUS_OFFSET[static_cast<uint8_t>(Machine_t::Kh910)] - GARTER_SLOP) - 1;

class FsmTest : public ::testing::Test {
protected:
  void SetUp() override {
    arduinoMock = arduinoMockInstance();
    serialMock = serialMockInstance();

    // pointers to global instances
    beeperMock = beeper;
    comMock = com;
    encodersMock = encoders;
    solenoidsMock = solenoids;

    opIdleMock = opIdle;
    opInitMock = opInit;
    opReadyMock = opReady;
    opTestMock = opTest;
    opErrorMock = opError;

    // The global instance does not get destroyed at the end of each test.
    // Ordinarily the mock instance would be local and such behaviour would
    // cause a memory leak. We must notify the test that this is not the case.
    Mock::AllowLeak(beeperMock);
    Mock::AllowLeak(comMock);
    Mock::AllowLeak(encodersMock);
    Mock::AllowLeak(solenoidsMock);

    Mock::AllowLeak(opIdleMock);
    Mock::AllowLeak(opInitMock);
    Mock::AllowLeak(opReadyMock);
    Mock::AllowLeak(opTestMock);
    Mock::AllowLeak(opErrorMock);

    // start in state `opIdle`
    fsm->init();
    expect_knit_init();
    opKnit->init();
    fsm->setMachineType(Machine_t::Kh910);
    expected_isready(Direction_t::NoDirection, Direction_t::NoDirection, 0);
  }

  void TearDown() override {
    releaseArduinoMock();
    releaseSerialMock();
  }

  ArduinoMock *arduinoMock;
  BeeperMock *beeperMock;
  ComMock *comMock;
  EncodersMock *encodersMock;
  SerialMock *serialMock;
  SolenoidsMock *solenoidsMock;

  OpIdleMock *opIdleMock;
  OpInitMock *opInitMock;
  OpReadyMock *opReadyMock;
  OpTestMock *opTestMock;
  OpErrorMock *opErrorMock;

  void expect_knit_init() {
    EXPECT_CALL(*arduinoMock, pinMode(ENC_PIN_A, INPUT));
    EXPECT_CALL(*arduinoMock, pinMode(ENC_PIN_B, INPUT));
    EXPECT_CALL(*arduinoMock, pinMode(ENC_PIN_C, INPUT));
    EXPECT_CALL(*arduinoMock, pinMode(LED_PIN_A, OUTPUT));
    EXPECT_CALL(*arduinoMock, pinMode(LED_PIN_B, OUTPUT));
    EXPECT_CALL(*arduinoMock, digitalWrite(LED_PIN_A, HIGH));
    EXPECT_CALL(*arduinoMock, digitalWrite(LED_PIN_B, HIGH));
    EXPECT_CALL(*solenoidsMock, init);
  }

  void expected_isready(Direction_t dir, Direction_t hall, uint8_t position) {
    fsm->m_direction = dir;
    fsm->m_hallActive = hall;
    fsm->m_position = position;
  }

  void expect_reqLine() {
    EXPECT_CALL(*comMock, send_reqLine);
  }

  void expect_indState() {
    EXPECT_CALL(*comMock, send_indState);
  }

  void expect_get_ready() {
    // start in state `OpInit`
    ASSERT_EQ(fsm->getState(), opInitMock);

    expect_indState();
    EXPECT_CALL(*solenoidsMock, setSolenoids(SOLENOIDS_BITMASK));
    EXPECT_CALL(*arduinoMock, digitalWrite(LED_PIN_A, LOW));
  }

  void expected_state(OpInterface *state) {
    fsm->setState(state);
    fsm->update();
  }

  void expected_update() {
    EXPECT_CALL(*encodersMock, getPosition).Times(1);
    EXPECT_CALL(*encodersMock, getDirection).Times(1);
    EXPECT_CALL(*encodersMock, getHallActive).Times(1);
    EXPECT_CALL(*encodersMock, getBeltShift).Times(1);
    EXPECT_CALL(*encodersMock, getCarriage).Times(1);
    fsm->update();

    // test expectations without destroying instance
    ASSERT_TRUE(Mock::VerifyAndClear(encodersMock));
  }

  void expected_update_idle() {
    // starts in state `OpIdle`
    ASSERT_EQ(fsm->getState(), opIdleMock);

    EXPECT_CALL(*opIdleMock, update);
    expected_update();

    // test expectations without destroying instance
    ASSERT_TRUE(Mock::VerifyAndClear(opIdleMock));
  }

  void expected_update_init() {
    // starts in state `OpInit`
    ASSERT_EQ(fsm->getState(), opInitMock);

    EXPECT_CALL(*opInitMock, update);
    expected_update();

    // test expectations without destroying instance
    ASSERT_TRUE(Mock::VerifyAndClear(opInitMock));
  }

  void expected_update_ready() {
    // starts in state `OpReady`
    ASSERT_EQ(fsm->getState(), opReadyMock);

    EXPECT_CALL(*opReadyMock, update);
    expected_update();

    // test expectations without destroying instance
    ASSERT_TRUE(Mock::VerifyAndClear(opReadyMock));
  }

  void expected_update_knit() {
    // starts in state `OpKnit`
    ASSERT_EQ(fsm->getState(), opKnit);

    expected_update();
  }

  void expected_update_test() {
    // starts in state `OpTest`
    ASSERT_EQ(fsm->getState(), opTestMock);

    EXPECT_CALL(*opTestMock, update);
    expected_update();

    // test expectations without destroying instance
    ASSERT_TRUE(Mock::VerifyAndClear(opTestMock));
  }

  void expected_update_error(unsigned long t) {
    // starts in state `OpError`
    ASSERT_EQ(fsm->getState(), opErrorMock);

    EXPECT_CALL(*arduinoMock, millis).WillOnce(Return(t));
    EXPECT_CALL(*opErrorMock, update);
    expected_update();

    // test expectations without destroying instance
    ASSERT_TRUE(Mock::VerifyAndClear(opErrorMock));
  }

  void expect_first_knit() {
    EXPECT_CALL(*arduinoMock, delay(START_KNITTING_DELAY));
    EXPECT_CALL(*beeperMock, finishedLine);
    expect_reqLine();
  }
};

TEST_F(FsmTest, test_setState) {
  fsm->setState(opInitMock);

  EXPECT_CALL(*opIdle, end);
  EXPECT_CALL(*opInit, begin);
  expected_update_idle();

  ASSERT_TRUE(fsm->getState() == opInitMock);

  // test expectations without destroying instance
  ASSERT_TRUE(Mock::VerifyAndClear(opIdleMock));
  ASSERT_TRUE(Mock::VerifyAndClear(opInitMock));
}

TEST_F(FsmTest, test_update_init) {
  // Get to state `OpInit`
  fsm->setState(opInitMock);
  expected_update_idle();
  ASSERT_EQ(fsm->getState(), opInitMock);

  // no transition to state `OpReady`
  expected_isready(Direction_t::Left, Direction_t::Left, 0);
  expected_update_init();
  ASSERT_TRUE(fsm->getState() == opInitMock);

  // no transition to state `OpReady`
  expected_isready(Direction_t::Right, Direction_t::Right, 0);
  expected_update_init();
  ASSERT_TRUE(fsm->getState() == opInitMock);

  // transition to state `OpReady`
  expected_isready(Direction_t::Left, Direction_t::Right, positionPassedRight);
  expect_get_ready();
  expected_update();
  ASSERT_EQ(fsm->getState(), opReadyMock);
/*
  // get to state `OpInit`
  fsm->setState(opInitMock);
  expected_update_ready();

  // transition to state `OpReady`
  expected_isready(Direction_t::Right, Direction_t::Left, positionPassedLeft);
  expect_get_ready();
  expected_update();
  ASSERT_TRUE(fsm->getState() == opReadyMock);
*/
  // test expectations without destroying instance
  ASSERT_TRUE(Mock::VerifyAndClear(comMock));
  ASSERT_TRUE(Mock::VerifyAndClear(solenoidsMock));
}

TEST_F(FsmTest, test_update_test) {
  // get in state `OpTest`
  fsm->setState(opTestMock);
  expected_update_idle();

  // now in state `OpTest`
  expected_update_test();

  // now quit test
  fsm->setState(opInitMock);
  expect_knit_init();
  expected_update_test();
  ASSERT_TRUE(fsm->getState() == opInitMock);

  // test expectations without destroying instance
  ASSERT_TRUE(Mock::VerifyAndClear(solenoidsMock));
}

TEST_F(FsmTest, test_update_knit) {
  // get to state `OpReady`
  fsm->setState(opReadyMock);
  expected_update_idle();

  // get to state `OpKnit`
  fsm->setState(opKnit);
  expected_update_ready();
  ASSERT_TRUE(fsm->getState() == opKnit);

  // now in state `OpKnit`
  expect_first_knit();
  expected_update_knit();

  // test expectations without destroying instance
  ASSERT_TRUE(Mock::VerifyAndClear(beeperMock));
  ASSERT_TRUE(Mock::VerifyAndClear(comMock));
}

TEST_F(FsmTest, test_update_error) {
  // get to state `OpError`
  fsm->setState(opErrorMock);
  expected_update_idle();

  // now in state `OpError`
  expected_update_error(0);

  // too soon to flash
  EXPECT_CALL(*arduinoMock, digitalWrite).Times(0);
  expected_update_error(FLASH_DELAY - 1);

  // flash first time
  EXPECT_CALL(*arduinoMock, digitalWrite(LED_PIN_A, LOW));
  EXPECT_CALL(*arduinoMock, digitalWrite(LED_PIN_B, HIGH));
  EXPECT_CALL(*comMock, send_indState);
  expected_update_error(FLASH_DELAY);

  // alternate flash
  EXPECT_CALL(*arduinoMock, digitalWrite(LED_PIN_A, HIGH));
  EXPECT_CALL(*arduinoMock, digitalWrite(LED_PIN_B, LOW));
  EXPECT_CALL(*comMock, send_indState);
  expected_update_error(2 * FLASH_DELAY);

  // get to state `OpInit`
  EXPECT_CALL(*arduinoMock, digitalWrite(LED_PIN_B, LOW));
  expect_knit_init();
  expected_state(opInitMock);

  // test expectations without destroying instance
  ASSERT_TRUE(Mock::VerifyAndClear(solenoidsMock));
}
/*
TEST_F(FsmTest, test_update_default) {
  // get to default state
  fsm->setState(static_cast<FsmState_t>(99));
  expected_update_idle();
  // ASSERT_TRUE(static_cast<uint8_t>(fsm->getState()) == 99); // FIXME

  // now in default state
  EXPECT_CALL(*arduinoMock, digitalWrite).Times(0);
  expected_update();
}
*/
