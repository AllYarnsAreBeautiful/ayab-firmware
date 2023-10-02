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
#include <opTest.h>

#include <controller_mock.h>
#include <opKnit_mock.h>

using ::testing::_;
using ::testing::An;
using ::testing::AtLeast;
using ::testing::Mock;
using ::testing::Return;

extern OpInit& opInit;
extern OpReady& opReady;
extern OpTest& opTest;

extern ControllerMock& controller;
extern OpKnitMock& opKnit;

class OpInitTest : public ::testing::Test {
protected:
  void SetUp() override {
    arduinoMock = arduinoMockInstance();

    // pointers to global instances
    controllerMock = &controller;
    opKnitMock = &opKnit;

    // The global instances do not get destroyed at the end of each test.
    // Ordinarily the mock instance would be local and such behaviour would
    // cause a memory leak. We must notify the test that this is not the case.
    Mock::AllowLeak(controllerMock);
    Mock::AllowLeak(opKnitMock);
  }

  void TearDown() override {
    releaseArduinoMock();
  }

  ArduinoMock *arduinoMock;
  ControllerMock *controllerMock;
  OpKnitMock *opKnitMock;

  uint8_t get_position_past_left(Machine_t m) {
    return (END_LEFT_PLUS_OFFSET[static_cast<uint8_t>(m)] + GARTER_SLOP) + 1;
  }

  uint8_t get_position_past_right(Machine_t m) {
    return (END_RIGHT_MINUS_OFFSET[static_cast<uint8_t>(m)] - GARTER_SLOP) - 1;
  }

  void expect_update(uint16_t pos, Direction_t dir, Direction_t hall) {
    EXPECT_CALL(*controllerMock, getPosition).WillRepeatedly(Return(pos));
    EXPECT_CALL(*controllerMock, getDirection).WillRepeatedly(Return(dir));
    EXPECT_CALL(*controllerMock, getHallActive).WillRepeatedly(Return(hall));
    EXPECT_CALL(*controllerMock, getMachineType).WillRepeatedly(Return(Machine_t::Kh910));
    EXPECT_CALL(*controllerMock, getState).WillRepeatedly(Return(&opInit));
  }

  void expect_ready(bool ready) {
    if (ready) {
      EXPECT_CALL(*controllerMock, state);
    }
    ASSERT_EQ(opInit.isReady(), ready);
  }
};

TEST_F(OpInitTest, test_state) {
  ASSERT_EQ(opInit.state(), OpState_t::Init);
}

TEST_F(OpInitTest, test_init) {
  // no calls expected
  opInit.init();
  ASSERT_EQ(opInit.m_lastHall, Direction_t::NoDirection);
}

TEST_F(OpInitTest, test_reqTest) {
  EXPECT_CALL(*controllerMock, setState(&opTest));
  const uint8_t buffer[] = {static_cast<uint8_t>(API_t::reqTest)};
  opInit.com(buffer, 1);

  // test expectations without destroying instance
  ASSERT_TRUE(Mock::VerifyAndClear(controllerMock));
}

TEST_F(OpInitTest, test_com_unrecognized) {
  // no calls expected
  const uint8_t buffer[] = {0xFF};
  opInit.com(buffer, 1);
}

TEST_F(OpInitTest, test_end) {
  // no calls expected
  opInit.end();
}

TEST_F(OpInitTest, test_begin910) {
  EXPECT_CALL(*controllerMock, getMachineType());
  EXPECT_CALL(*arduinoMock, digitalWrite(LED_PIN_A, LOW));
  opInit.begin();

  // test expectations without destroying instance
  ASSERT_TRUE(Mock::VerifyAndClear(controllerMock));
}

TEST_F(OpInitTest, test_updateF) {
  // isReady() == false
  expect_update(get_position_past_right(Machine_t::Kh910), Direction_t::Left, Direction_t::Left);
  EXPECT_CALL(*controllerMock, getState).Times(0);
  EXPECT_CALL(*controllerMock, setState(&opReady)).Times(0);
  opInit.update();

  // test expectations without destroying instance
  ASSERT_TRUE(Mock::VerifyAndClear(controllerMock));
}

TEST_F(OpInitTest, test_updateT) {
  // isReady() == true
  expect_update(get_position_past_left(Machine_t::Kh910), Direction_t::Right, Direction_t::Left);
  EXPECT_CALL(*controllerMock, state);
  EXPECT_CALL(*controllerMock, setState(&opReady));
  opInit.update();

  // test expectations without destroying instance
  ASSERT_TRUE(Mock::VerifyAndClear(controllerMock));
}

TEST_F(OpInitTest, test_op_init_RLL) {
  // not ready
  expect_update(get_position_past_right(Machine_t::Kh910), Direction_t::Left, Direction_t::Left);
  expect_ready(false);

  // test expectations without destroying instance
  ASSERT_TRUE(Mock::VerifyAndClear(controllerMock));
}

TEST_F(OpInitTest, test_op_init_LRR) {
  // not ready
  expect_update(get_position_past_left(Machine_t::Kh910), Direction_t::Right, Direction_t::Right);
  expect_ready(false);

  // test expectations without destroying instance
  ASSERT_TRUE(Mock::VerifyAndClear(controllerMock));
}

TEST_F(OpInitTest, test_op_init_LRL) {
  // Machine is initialized when Left hall sensor
  // is passed in Right direction inside active needles.
  expect_update(get_position_past_left(Machine_t::Kh910), Direction_t::Right, Direction_t::Left);
  expect_ready(true);

  // test expectations without destroying instance
  ASSERT_TRUE(Mock::VerifyAndClear(controllerMock));
}

TEST_F(OpInitTest, test_op_init_XRL) {
  // Machine is initialized when Left hall sensor
  // is passed in Right direction inside active needles.
  expect_update(get_position_past_left(Machine_t::Kh910) - 2, Direction_t::Right, Direction_t::Left);
  expect_ready(false);

  // test expectations without destroying instance
  ASSERT_TRUE(Mock::VerifyAndClear(controllerMock));
}

TEST_F(OpInitTest, test_op_init_XLR) {
  // New feature (August 2020): the machine is also initialized
  // when the right Hall sensor is passed in the Left direction.
  expect_update(get_position_past_right(Machine_t::Kh910) + 2, Direction_t::Left, Direction_t::Right);
  expect_ready(false);

  // test expectations without destroying instance
  ASSERT_TRUE(Mock::VerifyAndClear(controllerMock));
}

TEST_F(OpInitTest, test_op_init_RLR) {
  // New feature (August 2020): the machine is also initialized
  // when the right Hall sensor is passed in the Left direction.
  expect_update(get_position_past_right(Machine_t::Kh910), Direction_t::Left, Direction_t::Right);
  expect_ready(true);

  // test expectations without destroying instance
  ASSERT_TRUE(Mock::VerifyAndClear(controllerMock));
}

TEST_F(OpInitTest, test_op_init_RLN) {
  // not ready
  expect_update(get_position_past_right(Machine_t::Kh910), Direction_t::Left, Direction_t::NoDirection);
  expect_ready(false);

  // test expectations without destroying instance
  ASSERT_TRUE(Mock::VerifyAndClear(controllerMock));
}
