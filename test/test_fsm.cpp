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
 *    Modified Work Copyright 2020 Sturla Lange, Tom Price
 *    http://ayab-knitting.com
 */

#include <gtest/gtest.h>

#include <fsm.h>
#include <knitter.h>
#include <encoders.h>

#include <beeper_mock.h>
#include <com_mock.h>
#include <encoders_mock.h>
#include <solenoids_mock.h>
#include <tester_mock.h>

using ::testing::_;
using ::testing::AtLeast;
using ::testing::Mock;
using ::testing::Return;
using ::testing::Test;

extern Fsm *fsm;
extern Knitter *knitter;

extern BeeperMock *beeper;
extern ComMock *com;
extern EncodersMock *encoders;
extern SolenoidsMock *solenoids;
extern TesterMock *tester;

// Defaults for position
const uint8_t positionPassedLeft = (END_LEFT[Kh910] + END_OFFSET[Kh910] + GARTER_SLOP) + 1;
const uint8_t positionPassedRight = (END_RIGHT[Kh910] - END_OFFSET[Kh910] - GARTER_SLOP) - 1;

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
    testerMock = tester;

    // The global instance does not get destroyed at the end of each test.
    // Ordinarily the mock instance would be local and such behaviour would
    // cause a memory leak. We must notify the test that this is not the case.
    Mock::AllowLeak(beeperMock);
    Mock::AllowLeak(comMock);
    Mock::AllowLeak(encodersMock);
    Mock::AllowLeak(solenoidsMock);
    Mock::AllowLeak(testerMock);

    // start in state `s_init`
    EXPECT_CALL(*arduinoMock, millis);
    fsm->init();
    // expected_isr(NoDirection, NoDirection);
    // EXPECT_CALL(*arduinoMock, digitalWrite(LED_PIN_A, LOW));
    // fsm->setState(s_init);
    // EXPECT_CALL(*comMock, update);
    // fsm->dispatch();
    // ASSERT_TRUE(fsm->getState() == s_init);
    expect_knitter_init();
    knitter->init();
    knitter->setMachineType(Kh910);
    expected_isr(NoDirection, NoDirection, 0);
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
  TesterMock *testerMock;

  void expect_knitter_init() {
    EXPECT_CALL(*arduinoMock, pinMode(ENC_PIN_A, INPUT));
    EXPECT_CALL(*arduinoMock, pinMode(ENC_PIN_B, INPUT));
    EXPECT_CALL(*arduinoMock, pinMode(ENC_PIN_C, INPUT));
    EXPECT_CALL(*arduinoMock, pinMode(LED_PIN_A, OUTPUT));
    EXPECT_CALL(*arduinoMock, pinMode(LED_PIN_B, OUTPUT));
    EXPECT_CALL(*arduinoMock, digitalWrite(LED_PIN_A, HIGH));
    EXPECT_CALL(*arduinoMock, digitalWrite(LED_PIN_B, HIGH));
    EXPECT_CALL(*solenoidsMock, init);
  }

  void expected_isr(Direction_t dir, Direction_t hall, uint8_t position) {
    EXPECT_CALL(*encodersMock, encA_interrupt);
    EXPECT_CALL(*encodersMock, getPosition).WillRepeatedly(Return(position));
    EXPECT_CALL(*encodersMock, getDirection).WillRepeatedly(Return(dir));
    EXPECT_CALL(*encodersMock, getHallActive).WillRepeatedly(Return(hall));
    EXPECT_CALL(*encodersMock, getBeltShift).Times(AtLeast(1));
    EXPECT_CALL(*encodersMock, getCarriage).Times(AtLeast(1));
    knitter->isr();

    // test expectations without destroying instance
    ASSERT_TRUE(Mock::VerifyAndClear(encodersMock));
  }

  void expect_reqLine() {
    EXPECT_CALL(*comMock, send_reqLine);
  }

  void expect_indState() {
    EXPECT_CALL(*comMock, send_indState);
  }

  void expect_get_ready() {
    // start in state `s_init`
    ASSERT_TRUE(fsm->getState() == s_init);

    expect_indState();
    EXPECT_CALL(*solenoidsMock, setSolenoids(SOLENOIDS_BITMASK));
    EXPECT_CALL(*arduinoMock, digitalWrite(LED_PIN_A, LOW));
  }

  void get_in_ready() {
    expect_get_ready();
    expected_state(s_ready);

    // ends in state `s_ready`
    ASSERT_TRUE(fsm->getState() == s_ready);
  }

  void expected_state(OpState_t state) {
    fsm->setState(state);
    expected_dispatch();
  }

  void expected_dispatch() {
    EXPECT_CALL(*comMock, update);
    fsm->dispatch();

    // test expectations without destroying instance
    ASSERT_TRUE(Mock::VerifyAndClear(comMock));
  }

  void expected_dispatch_init() {
    // starts in state `s_init`
    ASSERT_TRUE(fsm->getState() == s_init);

    EXPECT_CALL(*arduinoMock, digitalWrite(LED_PIN_A, LOW));
    expected_dispatch();
  }

  void expected_dispatch_ready() {
    // starts in state `s_ready`
    ASSERT_TRUE(fsm->getState() == s_ready);

    EXPECT_CALL(*arduinoMock, digitalWrite(LED_PIN_A, LOW));
    expected_dispatch();
  }

  void expected_dispatch_knit() {
    // starts in state `s_knit`
    ASSERT_TRUE(fsm->getState() == s_knit);

    EXPECT_CALL(*arduinoMock, digitalWrite(LED_PIN_A, HIGH)); // green LED on
    expected_dispatch();
  }

  void expected_dispatch_test() {
    // starts in state `s_test`
    ASSERT_TRUE(fsm->getState() == s_test);

    EXPECT_CALL(*testerMock, loop);
    expected_dispatch();

    // test expectations without destroying instance
    ASSERT_TRUE(Mock::VerifyAndClear(testerMock));
  }

  void expected_dispatch_error(unsigned long t) {
    // starts in state `s_error`
    ASSERT_TRUE(fsm->getState() == s_error);

    EXPECT_CALL(*arduinoMock, millis).WillOnce(Return(t));
    expected_dispatch();
  }

  void expect_first_knit() {
    EXPECT_CALL(*arduinoMock, delay(2000));
    EXPECT_CALL(*beeperMock, finishedLine);
    expect_reqLine();
  }
};

TEST_F(FsmTest, test_setState) {
  fsm->setState(s_ready);
  expected_dispatch_init();
  ASSERT_TRUE(fsm->getState() == s_ready);
}

TEST_F(FsmTest, test_dispatch_init) {
  // no transition to state `s_ready`
  expected_isr(Left, Left, 0);
  expected_dispatch_init();
  ASSERT_TRUE(fsm->getState() == s_init);

  // no transition to state `s_ready`
  expected_isr(Right, Right, 0);
  expected_dispatch_init();
  ASSERT_TRUE(fsm->getState() == s_init);

  // transition to state `s_ready`
  expected_isr(Left, Right, positionPassedRight);
  expect_get_ready();
  expected_dispatch();
  ASSERT_EQ(fsm->getState(), s_ready);

  // get to state `s_init`
  fsm->setState(s_init);
  expected_dispatch_ready();

  // transition to state `s_ready`
  expected_isr(Right, Left, positionPassedLeft);
  expect_get_ready();
  expected_dispatch();
  ASSERT_TRUE(fsm->getState() == s_ready);

  // test expectations without destroying instance
  ASSERT_TRUE(Mock::VerifyAndClear(comMock));
  ASSERT_TRUE(Mock::VerifyAndClear(solenoidsMock));
}

TEST_F(FsmTest, test_dispatch_test) {
  // get in state `s_test`
  fsm->setState(s_test);
  expected_dispatch_init();

  // now in state `s_test`
  expected_dispatch_test();

  // now quit test
  fsm->setState(s_init);
  expect_knitter_init();
  expected_dispatch_test();
  ASSERT_TRUE(fsm->getState() == s_init);

  // test expectations without destroying instance
  ASSERT_TRUE(Mock::VerifyAndClear(solenoidsMock));
}

TEST_F(FsmTest, test_dispatch_knit) {
  // get to state `s_ready`
  fsm->setState(s_ready);
  expected_dispatch_init();

  // get to state `s_knit`
  fsm->setState(s_knit);
  expected_dispatch_ready();
  ASSERT_TRUE(fsm->getState() == s_knit);

  // now in state `s_knit`
  expect_first_knit();
  expected_dispatch_knit();

  // test expectations without destroying instance
  ASSERT_TRUE(Mock::VerifyAndClear(beeperMock));
  ASSERT_TRUE(Mock::VerifyAndClear(comMock));
}

TEST_F(FsmTest, test_dispatch_error) {
  // get to state `s_error`
  fsm->setState(s_error);
  expected_dispatch_init();

  // now in state `s_error`
  expected_dispatch_error(0);

  // too soon to flash
  EXPECT_CALL(*arduinoMock, digitalWrite).Times(0);
  expected_dispatch_error(499);

  // flash first time
  EXPECT_CALL(*arduinoMock, digitalWrite(LED_PIN_A, LOW));
  EXPECT_CALL(*arduinoMock, digitalWrite(LED_PIN_B, HIGH));
  EXPECT_CALL(*comMock, send_indState);
  expected_dispatch_error(500);

  // alternate flash
  EXPECT_CALL(*arduinoMock, digitalWrite(LED_PIN_A, HIGH));
  EXPECT_CALL(*arduinoMock, digitalWrite(LED_PIN_B, LOW));
  EXPECT_CALL(*comMock, send_indState);
  expected_dispatch_error(1000);

  // get to state `s_init`
  EXPECT_CALL(*arduinoMock, digitalWrite(LED_PIN_B, LOW));
  expect_knitter_init();
  expected_state(s_init);

  // test expectations without destroying instance
  ASSERT_TRUE(Mock::VerifyAndClear(solenoidsMock));
}

TEST_F(FsmTest, test_dispatch_default) {
  // get to default state
  fsm->setState(static_cast<OpState_t>(99));
  expected_dispatch_init();
  ASSERT_TRUE(static_cast<uint8_t>(fsm->getState()) == 99);

  // now in default state
  EXPECT_CALL(*arduinoMock, digitalWrite).Times(0);
  expected_dispatch();
}
