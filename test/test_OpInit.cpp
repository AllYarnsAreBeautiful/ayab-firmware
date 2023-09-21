/*!`
 * \file test_OpInit.cpp
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

#include <opInit.h>
#include <opReady.h>

#include <fsm_mock.h>
#include <opKnit_mock.h>

using ::testing::_;
using ::testing::An;
using ::testing::AtLeast;
using ::testing::Mock;
using ::testing::Return;

extern OpInit *opInit;
extern OpReady *opReady;

extern FsmMock *fsm;
extern OpKnitMock *opKnit;

class OpInitTest : public ::testing::Test {
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
  }

  void TearDown() override {
    releaseArduinoMock();
    releaseSerialMock();
  }

  ArduinoMock *arduinoMock;
  SerialMock *serialMock;
  FsmMock *fsmMock;
  OpKnitMock *opKnitMock;
};

TEST_F(OpInitTest, test_begin910) {
  EXPECT_CALL(*fsmMock, getMachineType());
  EXPECT_CALL(*arduinoMock, digitalWrite(LED_PIN_A, LOW));
  opInit->begin();
}

TEST_F(OpInitTest, test_update_not_ready) {
  EXPECT_CALL(*opKnitMock, isReady()).WillOnce(Return(false));
  EXPECT_CALL(*fsmMock, setState(opReady)).Times(0);
  opInit->update();

  // test expectations without destroying instance
  ASSERT_TRUE(Mock::VerifyAndClear(fsmMock));
  ASSERT_TRUE(Mock::VerifyAndClear(opKnitMock));
}

TEST_F(OpInitTest, test_update_ready) {
  EXPECT_CALL(*opKnitMock, isReady()).WillOnce(Return(true));
  EXPECT_CALL(*fsmMock, setState(opReady));
  opInit->update();

  // test expectations without destroying instance
  ASSERT_TRUE(Mock::VerifyAndClear(fsmMock));
  ASSERT_TRUE(Mock::VerifyAndClear(opKnitMock));
}
/*
  void expected_isready(Direction_t dir, Direction_t hall, uint8_t position) {
    fsm->m_direction = dir;
    fsm->m_hallActive = hall;
    fsm->m_position = position;
  }

TEST_F(FsmTest, test_update_init) {
  // Get to state `OpInit`
  fsm->setState(opInitMock);
  EXPECT_CALL(*opInit, begin);
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

  // get to state `OpInit`
  fsm->setState(opInitMock);
  expected_update_ready();

  // transition to state `OpReady`
  expected_isready(Direction_t::Right, Direction_t::Left, positionPassedLeft);
  expect_get_ready();
  expected_update();
  ASSERT_TRUE(fsm->getState() == opReadyMock);

  // test expectations without destroying instance
  ASSERT_TRUE(Mock::VerifyAndClear(comMock));
  ASSERT_TRUE(Mock::VerifyAndClear(solenoidsMock));
  ASSERT_TRUE(Mock::VerifyAndClear(opIdleMock));
  ASSERT_TRUE(Mock::VerifyAndClear(opInitMock));
}
*/
