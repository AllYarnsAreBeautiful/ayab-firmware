/*!`
 * \file test_OpReady.cpp
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

#include <opReady.h>
#include <opTest.h>

#include <controller_mock.h>
#include <opKnit_mock.h>

using ::testing::_;
using ::testing::An;
using ::testing::AtLeast;
using ::testing::Mock;
using ::testing::Return;

extern OpReady *opReady;
extern OpTest *opTest;

extern ControllerMock *controller;
extern OpKnitMock *opKnit;

class OpReadyTest : public ::testing::Test {
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
  ControllerMock *controllerMock;
  SerialMock *serialMock;
  OpKnitMock *opKnitMock;
};

TEST_F(OpReadyTest, test_state) {
  ASSERT_EQ(opReady->state(), OpState_t::Ready);
}

TEST_F(OpReadyTest, test_begin) {
  EXPECT_CALL(*arduinoMock, digitalWrite(LED_PIN_A, LOW));
  opReady->begin();
}

TEST_F(OpReadyTest, test_init) {
  // nothing
  opReady->init();
}

TEST_F(OpReadyTest, test_reqStart) {
  EXPECT_CALL(*opKnitMock, startKnitting);
  const uint8_t buffer[] = {static_cast<uint8_t>(API_t::reqStart), 0, 10, 1, 0x36};
  opReady->com(buffer, 5);
}

TEST_F(OpReadyTest, test_reqTest) {
  EXPECT_CALL(*controllerMock, setState(opTest));
  const uint8_t buffer[] = {static_cast<uint8_t>(API_t::reqTest)};
  opReady->com(buffer, 1);
}

TEST_F(OpReadyTest, test_unrecognized) {
  // nothing
  const uint8_t buffer[] = {0xFF};
  opReady->com(buffer, 1);
}

TEST_F(OpReadyTest, test_update) {
  // nothing
  opReady->update();
}

TEST_F(OpReadyTest, test_end) {
  // nothing
  opReady->end();
}
