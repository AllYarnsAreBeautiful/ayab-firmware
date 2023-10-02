/*!`
 * \file test_com.cpp
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
#include <com.h>

#include <opIdle.h>
#include <opInit.h>
#include <opTest.h>

#include <opKnit_mock.h>
#include <controller_mock.h>
#include <packetSerialWrapper_mock.h>

using ::testing::_;
using ::testing::AtLeast;
using ::testing::Mock;
using ::testing::Return;

extern Com& com;
extern Beeper& beeper;

extern OpIdle& opIdle;
extern OpInit& opInit;
extern OpTest& opTest;

extern ControllerMock& controller;
extern OpKnitMock& opKnit;
extern PacketSerialWrapperMock& packetSerialWrapper;

class ComTest : public ::testing::Test {
protected:
  void SetUp() override {
    arduinoMock = arduinoMockInstance();

    // pointer to global instance
    controllerMock = &controller;
    opKnitMock = &opKnit;
    packetSerialWrapperMock = &packetSerialWrapper;

    // The global instance does not get destroyed at the end of each test.
    // Ordinarily the mock instance would be local and such behaviour would
    // cause a memory leak. We must notify the test that this is not the case.
    Mock::AllowLeak(controllerMock);
    Mock::AllowLeak(opKnitMock);
    Mock::AllowLeak(packetSerialWrapperMock);

    beeper.init(true);
    expect_init();
    com.init();
    controllerMock->init();
  }

  void TearDown() override {
    releaseArduinoMock();
  }

  ArduinoMock *arduinoMock;
  ControllerMock *controllerMock;
  OpKnitMock *opKnitMock;
  PacketSerialWrapperMock *packetSerialWrapperMock;

  void expect_init() {
    EXPECT_CALL(*packetSerialWrapperMock, begin);
  }

  void expect_send(bool once) {
    if (once) {
      EXPECT_CALL(*packetSerialWrapperMock, send).Times(1);
    } else {
      EXPECT_CALL(*packetSerialWrapperMock, send).Times(AtLeast(2));
    }
  }

  void expected_write_onPacketReceived(uint8_t *buffer, size_t size,
                                       bool once) {
    expect_send(once);
    EXPECT_CALL(*controllerMock, getState).WillOnce(Return(&opTest));
    com.onPacketReceived(buffer, size);
  }

  void reqInit(Machine_t machine) {
    uint8_t buffer[] = {static_cast<uint8_t>(API_t::reqInit), static_cast<uint8_t>(machine), 0};
    buffer[2] = com.CRC8(buffer, 2);
    EXPECT_CALL(*controllerMock, setState(&opInit));
    expect_send(true);
    opIdle.com(buffer, sizeof(buffer));
  }
};

TEST_F(ComTest, test_reqInit_fail1) {
  uint8_t buffer[] = {static_cast<uint8_t>(API_t::reqInit), static_cast<uint8_t>(Machine_t::Kh930)};
  EXPECT_CALL(*controllerMock, setState(&opInit)).Times(0);
  expect_send(true);
  opIdle.com(buffer, sizeof(buffer));

  // test expectations without destroying instance
  ASSERT_TRUE(Mock::VerifyAndClear(packetSerialWrapperMock));
}

TEST_F(ComTest, test_reqInit_fail2) {
  uint8_t buffer[] = {static_cast<uint8_t>(API_t::reqInit), static_cast<uint8_t>(Machine_t::Kh930), 0};
  buffer[2] = com.CRC8(buffer, 2) ^ 1;
  EXPECT_CALL(*controllerMock, setState(&opInit)).Times(0);
  expect_send(true);
  opIdle.com(buffer, sizeof(buffer));

  // test expectations without destroying instance
  ASSERT_TRUE(Mock::VerifyAndClear(packetSerialWrapperMock));
}

TEST_F(ComTest, test_reqInit_fail3) {
  uint8_t buffer[] = {static_cast<uint8_t>(API_t::reqInit), static_cast<uint8_t>(Machine_t::NoMachine), 0};
  buffer[2] = com.CRC8(buffer, 2);
  EXPECT_CALL(*controllerMock, setState(&opInit)).Times(0);
  expect_send(true);
  opIdle.com(buffer, sizeof(buffer));

  // test expectations without destroying instance
  ASSERT_TRUE(Mock::VerifyAndClear(packetSerialWrapperMock));
}

/*
TEST_F(ComTest, test_API) {
  ASSERT_EQ(API_VERSION, 6);
}
*/

/*
TEST_F(ComTest, test_reqtest_fail) {
  // no machineType
  uint8_t buffer[] = {static_cast<uint8_t>(API_t::reqTest)};
  EXPECT_CALL(*controllerMock, setState(&opTest)).Times(0);
  expected_write_onPacketReceived(buffer, sizeof(buffer), true);

  // test expectations without destroying instance
  ASSERT_TRUE(Mock::VerifyAndClear(controllerMock));
}
*/

TEST_F(ComTest, test_reqtest) {
  EXPECT_CALL(*controllerMock, setState(&opTest));
  expect_send(true);
  com.h_reqTest();

  // test expectations without destroying instance
  ASSERT_TRUE(Mock::VerifyAndClear(controllerMock));
  ASSERT_TRUE(Mock::VerifyAndClear(packetSerialWrapperMock));
}

TEST_F(ComTest, test_reqstart_fail1) {
  // checksum wrong
  uint8_t buffer[] = {static_cast<uint8_t>(API_t::reqStart), 0, 10, 1, 0x73};
  EXPECT_CALL(*opKnitMock, startKnitting).Times(0);
  expect_send(true);
  com.h_reqStart(buffer, sizeof(buffer));

  // test expectations without destroying instance
  ASSERT_TRUE(Mock::VerifyAndClear(opKnitMock));
  ASSERT_TRUE(Mock::VerifyAndClear(packetSerialWrapperMock));
}

TEST_F(ComTest, test_reqstart_fail2) {
  // not enough bytes
  uint8_t buffer[] = {static_cast<uint8_t>(API_t::reqStart), 0, 1, 0x74};
  EXPECT_CALL(*opKnitMock, startKnitting).Times(0);
  expect_send(true);
  com.h_reqStart(buffer, sizeof(buffer) - 1);

  // test expectations without destroying instance
  ASSERT_TRUE(Mock::VerifyAndClear(opKnitMock));
  ASSERT_TRUE(Mock::VerifyAndClear(packetSerialWrapperMock));
}

TEST_F(ComTest, test_reqstart_success_KH910) {
  reqInit(Machine_t::Kh910);
  uint8_t buffer[] = {static_cast<uint8_t>(API_t::reqStart), 0, 10, 1, 0x36};
  EXPECT_CALL(*opKnitMock, startKnitting);
  expect_send(true);
  com.h_reqStart(buffer, sizeof(buffer));

  // test expectations without destroying instance
  ASSERT_TRUE(Mock::VerifyAndClear(opKnitMock));
  ASSERT_TRUE(Mock::VerifyAndClear(packetSerialWrapperMock));
}

TEST_F(ComTest, test_reqstart_success_KH270) {
  reqInit(Machine_t::Kh270);
  uint8_t buffer[] = {static_cast<uint8_t>(API_t::reqStart), 0, 10, 1, 0x36};
  EXPECT_CALL(*opKnitMock, startKnitting);
  expect_send(true);
  com.h_reqStart(buffer, sizeof(buffer));

  // test expectations without destroying instance
  ASSERT_TRUE(Mock::VerifyAndClear(opKnitMock));
  ASSERT_TRUE(Mock::VerifyAndClear(packetSerialWrapperMock));
}

TEST_F(ComTest, test_reqinfo) {
  expect_send(true);
  com.h_reqInfo();

  // test expectations without destroying instance
  ASSERT_TRUE(Mock::VerifyAndClear(packetSerialWrapperMock));
}

TEST_F(ComTest, test_helpCmd) {
  uint8_t buffer[] = {static_cast<uint8_t>(API_t::helpCmd)};
  expected_write_onPacketReceived(buffer, sizeof(buffer), false);

  // test expectations without destroying instance
  ASSERT_TRUE(Mock::VerifyAndClear(packetSerialWrapperMock));
  ASSERT_TRUE(Mock::VerifyAndClear(controllerMock));
}

TEST_F(ComTest, test_sendCmd) {
  uint8_t buffer[] = {static_cast<uint8_t>(API_t::sendCmd)};
  expected_write_onPacketReceived(buffer, sizeof(buffer), false);

  // test expectations without destroying instance
  ASSERT_TRUE(Mock::VerifyAndClear(packetSerialWrapperMock));
  ASSERT_TRUE(Mock::VerifyAndClear(controllerMock));
}

TEST_F(ComTest, test_beepCmd) {
  uint8_t buffer[] = {static_cast<uint8_t>(API_t::beepCmd)};
  expected_write_onPacketReceived(buffer, sizeof(buffer), true);
  EXPECT_CALL(*arduinoMock, millis).WillOnce(Return(0U));
  beeper.update();
  EXPECT_CALL(*arduinoMock, analogWrite(PIEZO_PIN, BEEP_ON_DUTY));
  EXPECT_CALL(*arduinoMock, millis).WillOnce(Return(1U));
  beeper.update();

  // test expectations without destroying instance
  ASSERT_TRUE(Mock::VerifyAndClear(packetSerialWrapperMock));
  ASSERT_TRUE(Mock::VerifyAndClear(controllerMock));
}

TEST_F(ComTest, test_setSingleCmd) {
  uint8_t buffer[] = {static_cast<uint8_t>(API_t::setSingleCmd), 0, 0};
  expected_write_onPacketReceived(buffer, sizeof(buffer), true);

  // test expectations without destroying instance
  ASSERT_TRUE(Mock::VerifyAndClear(packetSerialWrapperMock));
  ASSERT_TRUE(Mock::VerifyAndClear(controllerMock));
}

TEST_F(ComTest, test_setAllCmd) {
  uint8_t buffer[] = {static_cast<uint8_t>(API_t::setAllCmd), 0, 0};
  expected_write_onPacketReceived(buffer, sizeof(buffer), true);

  // test expectations without destroying instance
  ASSERT_TRUE(Mock::VerifyAndClear(packetSerialWrapperMock));
  ASSERT_TRUE(Mock::VerifyAndClear(controllerMock));
}

TEST_F(ComTest, test_readEOLsensorsCmd) {
  uint8_t buffer[] = {static_cast<uint8_t>(API_t::readEOLsensorsCmd)};
  EXPECT_CALL(*arduinoMock, analogRead(EOL_PIN_L));
  EXPECT_CALL(*arduinoMock, analogRead(EOL_PIN_R));
  expected_write_onPacketReceived(buffer, sizeof(buffer), false);

  // test expectations without destroying instance
  ASSERT_TRUE(Mock::VerifyAndClear(packetSerialWrapperMock));
  ASSERT_TRUE(Mock::VerifyAndClear(controllerMock));
}

TEST_F(ComTest, test_readEncodersCmd) {
  uint8_t buffer[] = {static_cast<uint8_t>(API_t::readEncodersCmd)};
  EXPECT_CALL(*arduinoMock, digitalRead(ENC_PIN_A));
  EXPECT_CALL(*arduinoMock, digitalRead(ENC_PIN_B));
  EXPECT_CALL(*arduinoMock, digitalRead(ENC_PIN_C));
  expected_write_onPacketReceived(buffer, sizeof(buffer), false);

  // test expectations without destroying instance
  ASSERT_TRUE(Mock::VerifyAndClear(packetSerialWrapperMock));
  ASSERT_TRUE(Mock::VerifyAndClear(controllerMock));
}

TEST_F(ComTest, test_autoReadCmd) {
  uint8_t buffer[] = {static_cast<uint8_t>(API_t::autoReadCmd)};
  expected_write_onPacketReceived(buffer, sizeof(buffer), true);

  // test expectations without destroying instance
  ASSERT_TRUE(Mock::VerifyAndClear(packetSerialWrapperMock));
  ASSERT_TRUE(Mock::VerifyAndClear(controllerMock));
}

TEST_F(ComTest, test_autoTestCmd) {
  uint8_t buffer[] = {static_cast<uint8_t>(API_t::autoTestCmd)};
  expected_write_onPacketReceived(buffer, sizeof(buffer), true);

  // test expectations without destroying instance
  ASSERT_TRUE(Mock::VerifyAndClear(packetSerialWrapperMock));
  ASSERT_TRUE(Mock::VerifyAndClear(controllerMock));
}

/*
TEST_F(ComTest, test_stopCmd) {
  uint8_t buffer[] = {static_cast<uint8_t>(API_t::stopCmd)};
  com.onPacketReceived(buffer, sizeof(buffer));

  // test expectations without destroying instance
  ASSERT_TRUE(Mock::VerifyAndClear(packetSerialWrapperMock));
  ASSERT_TRUE(Mock::VerifyAndClear(controllerMock));
}
*/

/*
TEST_F(ComTest, test_quitCmd) {
  EXPECT_CALL(*controllerMock, setState(&opInit));
  EXPECT_CALL(*opKnitMock, init);
  com.h_quitCmd();

  // test expectations without destroying instance
  ASSERT_TRUE(Mock::VerifyAndClear(opKnitMock));
  ASSERT_TRUE(Mock::VerifyAndClear(controllerMock));
}
*/

TEST_F(ComTest, test_cnfline_kh910) {
  // dummy pattern
  uint8_t pattern[] = {1};

  // message for machine with 200 needles
  uint8_t buffer[30] = {static_cast<uint8_t>(API_t::cnfLine) /* 0x42 */,
                        0, 0, 1,
                        0xDE, 0xAD, 0xBE, 0xEF, 0x00,
                        0x00, 0x00, 0x00, 0x00, 0x00,
                        0x00, 0x00, 0x00, 0x00, 0x00,
                        0x00, 0x00, 0x00, 0x00, 0x00,
                        0x00, 0x00, 0x00, 0x00, 0x00,
                        0xA7}; // CRC8

  // start job
  reqInit(Machine_t::Kh910);
  opKnitMock->begin();
  opKnitMock->startKnitting(0, 199, pattern, false);

  // first call increments line number to zero, not accepted
  EXPECT_CALL(*opKnitMock, setNextLine).WillOnce(Return(false));
  EXPECT_CALL(*opKnitMock, setLastLine).Times(0);
  com.h_cnfLine(buffer, sizeof(buffer));

  // second call Line accepted, last line
  EXPECT_CALL(*opKnitMock, setNextLine).WillOnce(Return(true));
  EXPECT_CALL(*opKnitMock, setLastLine).Times(1);
  com.h_cnfLine(buffer, sizeof(buffer));

  // not last line
  buffer[3] = 0x00;
  buffer[29] = 0xC0;
  EXPECT_CALL(*opKnitMock, setNextLine).WillOnce(Return(true));
  EXPECT_CALL(*opKnitMock, setLastLine).Times(0);
  com.h_cnfLine(buffer, sizeof(buffer));

  // checksum wrong
  EXPECT_CALL(*opKnitMock, setNextLine).Times(0);
  buffer[29]--;
  com.h_cnfLine(buffer, sizeof(buffer));

  // not enough bytes in buffer
  EXPECT_CALL(*opKnitMock, setNextLine).Times(0);
  com.h_cnfLine(buffer, sizeof(buffer) - 1);

  // test expectations without destroying instance
  ASSERT_TRUE(Mock::VerifyAndClear(opKnitMock));
}

/*
TEST_F(ComTest, test_cnfline_kh270) {
  // dummy pattern
  uint8_t pattern[] = {1};

  // message for KH270
  // CRC8 calculated with
  // http://tomeko.net/online_tools/crc8.php?lang=en
  uint8_t buffer[20] = {static_cast<uint8_t>(API_t::cnfLine),
                        0, 0, 1,
                        0xDE, 0xAD, 0xBE, 0xEF, 0x00,
                        0x00, 0x00, 0x00, 0x00, 0x00,
                        0x00, 0x00, 0x00, 0x00, 0x00,
                        0xA7}; // CRC8

  // start job
  reqInit(Machine_t::Kh270);
  opKnitMock->begin();
  opKnitMock->startKnitting(0, 113, pattern, false);

  // Last line accepted
  EXPECT_CALL(*opKnitMock, setNextLine).WillOnce(Return(true));
  EXPECT_CALL(*opKnitMock, setLastLine).Times(1);
  com.h_cnfLine(buffer, sizeof(buffer));
}
*/

/*
TEST_F(ComTest, test_debug) {
  uint8_t buffer[] = {static_cast<uint8_t>(API_t::debug)};
  com.onPacketReceived(buffer, sizeof(buffer));
  ASSERT_TRUE(Mock::VerifyAndClear(controllerMock));
}
*/

TEST_F(ComTest, test_update) {
  EXPECT_CALL(*packetSerialWrapperMock, update);
  com.update();

  // test expectations without destroying instance
  ASSERT_TRUE(Mock::VerifyAndClear(packetSerialWrapperMock));
}

TEST_F(ComTest, test_send) {
  expect_send(true);
  uint8_t p[] = {1, 2, 3};
  com.send(p, 3);

  // test expectations without destroying instance
  ASSERT_TRUE(Mock::VerifyAndClear(packetSerialWrapperMock));
}

TEST_F(ComTest, test_sendMsg1) {
  expect_send(true);
  com.sendMsg(API_t::testRes, "abc");

  // test expectations without destroying instance
  ASSERT_TRUE(Mock::VerifyAndClear(packetSerialWrapperMock));
}

TEST_F(ComTest, test_sendMsg2) {
  char buf[] = "abc\0";
  expect_send(true);
  com.sendMsg(API_t::testRes, buf);

  // test expectations without destroying instance
  ASSERT_TRUE(Mock::VerifyAndClear(packetSerialWrapperMock));
}

TEST_F(ComTest, test_send_reqLine) {
  expect_send(true);
  com.send_reqLine(0);

  // test expectations without destroying instance
  ASSERT_TRUE(Mock::VerifyAndClear(packetSerialWrapperMock));
}

TEST_F(ComTest, test_send_indState) {
  EXPECT_CALL(*arduinoMock, analogRead(EOL_PIN_L));
  EXPECT_CALL(*arduinoMock, analogRead(EOL_PIN_R));
  EXPECT_CALL(*controllerMock, getState).WillOnce(Return(&opInit));
  EXPECT_CALL(*controllerMock, getCarriage);
  EXPECT_CALL(*controllerMock, getPosition);
  EXPECT_CALL(*controllerMock, getDirection);
  expect_send(true);
  com.send_indState(Err_t::Success);

  // test expectations without destroying instance
  ASSERT_TRUE(Mock::VerifyAndClear(controllerMock));
  ASSERT_TRUE(Mock::VerifyAndClear(packetSerialWrapperMock));
}
