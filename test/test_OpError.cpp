/*!`
 * \file test_OpError.cpp
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

#include <opError.h>

#include <controller_mock.h>
#include <opKnit_mock.h>

using ::testing::_;
using ::testing::An;
using ::testing::AtLeast;
using ::testing::Mock;
using ::testing::Return;

extern OpError *opError;

extern ControllerMock *controller;
extern OpKnitMock *opKnit;

class OpErrorTest : public ::testing::Test {
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
  }

  void TearDown() override {
    releaseArduinoMock();
    releaseSerialMock();
  }

  ArduinoMock *arduinoMock;
  SerialMock *serialMock;
  ControllerMock *controllerMock;
  OpKnitMock *opKnitMock;
};

TEST_F(OpErrorTest, test_state) {
  ASSERT_EQ(opError->state(), OpState_t::Error);
}

TEST_F(OpErrorTest, test_begin) {
  EXPECT_CALL(*arduinoMock, millis);
  opError->begin();
}

TEST_F(OpErrorTest, test_init) {
  // no calls expected
  opError->init();
}

TEST_F(OpErrorTest, test_com) {
  // no calls expected
  const uint8_t *buffer = {};
  opError->com(buffer, 0);
}

TEST_F(OpErrorTest, test_end) {
  EXPECT_CALL(*arduinoMock, digitalWrite(LED_PIN_A, LOW));
  EXPECT_CALL(*arduinoMock, digitalWrite(LED_PIN_B, LOW));
  EXPECT_CALL(*opKnitMock, init());
  opError->end();
}

TEST_F(OpErrorTest, test_update) {
  EXPECT_CALL(*arduinoMock, millis).WillOnce(Return(0U));
  opError->begin();

  // too soon to flash
  EXPECT_CALL(*arduinoMock, millis).WillOnce(Return(FLASH_DELAY - 1));
  EXPECT_CALL(*arduinoMock, digitalWrite).Times(0);
  opError->update();

  // flash first time
  EXPECT_CALL(*arduinoMock, millis).WillOnce(Return(FLASH_DELAY));
  EXPECT_CALL(*arduinoMock, digitalWrite(LED_PIN_A, LOW));
  EXPECT_CALL(*arduinoMock, digitalWrite(LED_PIN_B, HIGH));
  // send_indState
  EXPECT_CALL(*controllerMock, getState).WillOnce(Return(opError));
  EXPECT_CALL(*controllerMock, getCarriage);
  EXPECT_CALL(*controllerMock, getPosition);
  EXPECT_CALL(*controllerMock, getDirection);
  opError->update();

  // alternate flash
  EXPECT_CALL(*arduinoMock, millis).WillOnce(Return(FLASH_DELAY * 2));
  EXPECT_CALL(*arduinoMock, digitalWrite(LED_PIN_A, HIGH));
  EXPECT_CALL(*arduinoMock, digitalWrite(LED_PIN_B, LOW));
  // send_indState
  EXPECT_CALL(*controllerMock, getState).WillOnce(Return(opError));
  EXPECT_CALL(*controllerMock, getCarriage);
  EXPECT_CALL(*controllerMock, getPosition);
  EXPECT_CALL(*controllerMock, getDirection);
  opError->update();

  // test expectations without destroying instance
  ASSERT_TRUE(Mock::VerifyAndClear(controllerMock));
}
