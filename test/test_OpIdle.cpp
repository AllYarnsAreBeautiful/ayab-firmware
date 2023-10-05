/*!`
 * \file test_OpIdle.cpp
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

#include <com.h>
#include <opIdle.h>

#include <controller_mock.h>
#include <opKnit_mock.h>

using ::testing::_;
using ::testing::An;
using ::testing::AtLeast;
using ::testing::Mock;
using ::testing::Return;

extern OpIdle *opIdle;

extern ControllerMock *controller;
extern OpKnitMock *opKnit;

class OpIdleTest : public ::testing::Test {
protected:
  void SetUp() override {
    arduinoMock = arduinoMockInstance();

    // pointers to global instances
    controllerMock = controller;
    opKnitMock = opKnit;

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
};

TEST_F(OpIdleTest, test_state) {
  ASSERT_EQ(opIdle->state(), OpState_t::Idle);
}

TEST_F(OpIdleTest, test_begin) {
  EXPECT_CALL(*arduinoMock, digitalWrite(LED_PIN_A, LOW));
  opIdle->begin();
}

TEST_F(OpIdleTest, test_init) {
  // no calls expected
  opIdle->init();
}

TEST_F(OpIdleTest, test_reqTest) {
  // no calls expected
  // can't enter state `OpTest` from state `OpIdle`
  const uint8_t buffer[] = {static_cast<uint8_t>(API_t::reqTest)};
  opIdle->com(buffer, 1);
}

TEST_F(OpIdleTest, test_unrecognized) {
  // no calls expected
  const uint8_t buffer[] = {0xFF};
  opIdle->com(buffer, 1);
}

TEST_F(OpIdleTest, test_update) {
  // no calls expected
  opIdle->update();
}

TEST_F(OpIdleTest, test_end) {
  // no calls expected
  opIdle->end();
}
