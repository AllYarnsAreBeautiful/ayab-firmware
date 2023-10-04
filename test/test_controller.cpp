/*!`
 * \file test_controller.cpp
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

#include <controller.h>
#include <encoders.h>
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

extern Controller *controller;
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

class ControllerTest : public ::testing::Test {
protected:
  void SetUp() override {
    arduinoMock = arduinoMockInstance();

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

    // start in state `OpIdle`
    controller->init();
    opKnit->init();
    controller->setMachineType(Machine_t::Kh910);
    expected_isready(Direction_t::NoDirection, Direction_t::NoDirection, 0);
  }

  void TearDown() override {
    releaseArduinoMock();
  }

  ArduinoMock *arduinoMock;
  BeeperMock *beeperMock;
  ComMock *comMock;
  EncodersMock *encodersMock;
  SolenoidsMock *solenoidsMock;

  OpIdleMock *opIdleMock;
  OpInitMock *opInitMock;
  OpReadyMock *opReadyMock;
  OpTestMock *opTestMock;
  OpErrorMock *opErrorMock;

  void expect_reqLine() {
    EXPECT_CALL(*comMock, send_reqLine);
  }

  void expect_indState() {
    EXPECT_CALL(*comMock, send_indState);
  }

  void expected_isready(Direction_t dir, Direction_t hall, uint8_t position) {
    controller->m_direction = dir;
    controller->m_hallActive = hall;
    controller->m_position = position;
  }

  void expected_state(OpInterface *state) {
    controller->setState(state);
    controller->update();
  }

  void expected_update() {
    EXPECT_CALL(*encodersMock, getPosition).Times(1);
    EXPECT_CALL(*encodersMock, getDirection).Times(1);
    EXPECT_CALL(*encodersMock, getHallActive).Times(1);
    EXPECT_CALL(*encodersMock, getBeltShift).Times(1);
    EXPECT_CALL(*encodersMock, getCarriage).Times(1);
    controller->update();

    // test expectations without destroying instance
    ASSERT_TRUE(Mock::VerifyAndClear(encodersMock));
  }

  void expected_update_idle() {
    // starts in state `OpIdle`
    ASSERT_EQ(controller->getState(), opIdleMock);

    EXPECT_CALL(*opIdleMock, update);
    expected_update();

    // test expectations without destroying instance
    ASSERT_TRUE(Mock::VerifyAndClear(opIdleMock));
  }

  void expected_update_init() {
    // starts in state `OpInit`
    ASSERT_EQ(controller->getState(), opInitMock);

    EXPECT_CALL(*opInitMock, update);
    expected_update();

    // test expectations without destroying instance
    ASSERT_TRUE(Mock::VerifyAndClear(opInitMock));
  }

  void expected_update_ready() {
    // starts in state `OpReady`
    ASSERT_EQ(controller->getState(), opReadyMock);

    EXPECT_CALL(*opReadyMock, update);
    expected_update();

    // test expectations without destroying instance
    ASSERT_TRUE(Mock::VerifyAndClear(opReadyMock));
  }

  void expected_update_knit() {
    // starts in state `OpKnit`
    ASSERT_EQ(controller->getState(), opKnit);

    expected_update();
  }

  void expected_update_test() {
    // starts in state `OpTest`
    ASSERT_EQ(controller->getState(), opTestMock);

    EXPECT_CALL(*opTestMock, update);
    expected_update();

    // test expectations without destroying instance
    ASSERT_TRUE(Mock::VerifyAndClear(opTestMock));
  }

  void expect_first_knit() {
    EXPECT_CALL(*arduinoMock, delay(START_KNITTING_DELAY));
    EXPECT_CALL(*beeperMock, finishedLine);
    expect_reqLine();
  }
};

TEST_F(ControllerTest, test_init) {
  EXPECT_CALL(*arduinoMock, pinMode(ENC_PIN_A, INPUT));
  EXPECT_CALL(*arduinoMock, pinMode(ENC_PIN_B, INPUT));
  EXPECT_CALL(*arduinoMock, pinMode(ENC_PIN_C, INPUT));
  EXPECT_CALL(*arduinoMock, pinMode(LED_PIN_A, OUTPUT));
  EXPECT_CALL(*arduinoMock, pinMode(LED_PIN_B, OUTPUT));
  EXPECT_CALL(*arduinoMock, digitalWrite(LED_PIN_A, HIGH));
  EXPECT_CALL(*arduinoMock, digitalWrite(LED_PIN_B, HIGH));
  controller->init();
}

TEST_F(ControllerTest, test_setState) {
  controller->setState(opInitMock);

  EXPECT_CALL(*opIdle, end);
  EXPECT_CALL(*opInit, begin);
  expected_update_idle();
  ASSERT_EQ(controller->getState(), opInitMock);

  EXPECT_CALL(*opInitMock, state).WillOnce(Return(OpState_t::Init));
  controller->getState()->state();

  // test expectations without destroying instance
  ASSERT_TRUE(Mock::VerifyAndClear(opIdleMock));
  ASSERT_TRUE(Mock::VerifyAndClear(opInitMock));
}

TEST_F(ControllerTest, test_getHallActive) {
  controller->init();
  ASSERT_EQ(controller->getHallActive(), Direction_t::NoDirection);
}

TEST_F(ControllerTest, test_ready_state) {
  controller->setState(opReadyMock);
  expected_update_idle();
  ASSERT_EQ(controller->getState(), opReadyMock);

  EXPECT_CALL(*opReadyMock, state).WillOnce(Return(OpState_t::Ready));
  controller->getState()->state();
}

TEST_F(ControllerTest, test_update_knit) {
  // get to state `OpReady`
  controller->setState(opReadyMock);
  expected_update_idle();

  // get to state `OpKnit`
  controller->setState(opKnit);
  expected_update_ready();
  ASSERT_EQ(controller->getState(), opKnit);

  // now in state `OpKnit`
  expect_first_knit();
  expected_update_knit();

  // test expectations without destroying instance
  ASSERT_TRUE(Mock::VerifyAndClear(beeperMock));
  ASSERT_TRUE(Mock::VerifyAndClear(comMock));
}

TEST_F(ControllerTest, test_update_test) {
  // get in state `OpTest`
  controller->setState(opTestMock);
  expected_update_idle();

  // now in state `OpTest`
  expected_update_test();
  ASSERT_EQ(controller->getState(), opTestMock);

  EXPECT_CALL(*opTestMock, state).WillOnce(Return(OpState_t::Test));
  controller->getState()->state();

  // now quit test
  controller->setState(opInitMock);
  EXPECT_CALL(*opTestMock, end);
  EXPECT_CALL(*opInitMock, begin);
  expected_update_test();
  ASSERT_EQ(controller->getState(), opInitMock);

  // test expectations without destroying instance
  ASSERT_TRUE(Mock::VerifyAndClear(opInitMock));
  ASSERT_TRUE(Mock::VerifyAndClear(opTestMock));
}

TEST_F(ControllerTest, test_com) {
  const uint8_t buffer[] = {0xFF};
  EXPECT_CALL(*opIdleMock, com);
  controller->com(buffer, 1);

  // test expectations without destroying instance
  ASSERT_TRUE(Mock::VerifyAndClear(opIdleMock));
}

TEST_F(ControllerTest, test_error_state) {
  controller->setState(opErrorMock);
  expected_update_idle();
  ASSERT_EQ(controller->getState(), opErrorMock);

  EXPECT_CALL(*opErrorMock, state).WillOnce(Return(OpState_t::Error));
  controller->getState()->state();
}
