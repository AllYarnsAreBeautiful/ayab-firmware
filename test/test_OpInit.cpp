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

TEST_F(OpInitTest, test_state) {
  ASSERT_EQ(opInit->state(), OpState_t::Init);
}

TEST_F(OpInitTest, test_init) {
  // nothing
  opInit->init();
}

TEST_F(OpInitTest, test_com) {
  // nothing
  const uint8_t *buffer = {};
  opInit->com(buffer, 0);
}

TEST_F(OpInitTest, test_end) {
  // nothing
  opInit->end();
}

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
