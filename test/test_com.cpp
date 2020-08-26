/*!`
 * \file test_serial_encoding.cpp
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

#include <com.h>
#include <encoders.h>

#include <fsm_mock.h>
#include <knitter_mock.h>

using ::testing::_;
using ::testing::AtLeast;
using ::testing::Mock;
using ::testing::Return;

extern Com *com;

extern FsmMock *fsm;
extern KnitterMock *knitter;

class ComTest : public ::testing::Test {
protected:
  void SetUp() override {
    arduinoMock = arduinoMockInstance();
    serialMock = serialMockInstance();

    // pointer to global instance
    fsmMock = fsm;
    knitterMock = knitter;

    // The global instance does not get destroyed at the end of each test.
    // Ordinarily the mock instance would be local and such behaviour would
    // cause a memory leak. We must notify the test that this is not the case.
    Mock::AllowLeak(fsmMock);
    Mock::AllowLeak(knitterMock);

    expect_init();
    com->init();
  }

  void TearDown() override {
    releaseArduinoMock();
    releaseSerialMock();
  }

  ArduinoMock *arduinoMock;
  FsmMock *fsmMock;
  KnitterMock *knitterMock;
  SerialMock *serialMock;

  void expect_init() {
    EXPECT_CALL(*serialMock, begin);
  }

  void expect_write(bool once) {
    if (once) {
      EXPECT_CALL(*serialMock, write(_, _));
      EXPECT_CALL(*serialMock, write(SLIP::END));
    } else {
      EXPECT_CALL(*serialMock, write(_, _)).Times(AtLeast(1));
      EXPECT_CALL(*serialMock, write(SLIP::END)).Times(AtLeast(1));
    }
  }

  void expected_write_onPacketReceived(uint8_t *buffer, size_t size,
                                       bool once) {
    expect_write(once);
    com->onPacketReceived(buffer, size);
  }
};

/*
TEST_F(ComTest, test_API) {
  ASSERT_EQ(API_VERSION, 6);
}
*/

TEST_F(ComTest, test_reqtest_fail) {
  // no machineType
  uint8_t buffer[] = {reqTest_msgid};
  EXPECT_CALL(*fsmMock, setState(s_test)).Times(0);
  expected_write_onPacketReceived(buffer, sizeof(buffer), true);

  // test expectations without destroying instance
  ASSERT_TRUE(Mock::VerifyAndClear(fsmMock));
}

TEST_F(ComTest, test_reqtest_success_KH270) {
  uint8_t buffer[] = {reqTest_msgid, Kh270};
  EXPECT_CALL(*fsmMock, setState(s_test));
  EXPECT_CALL(*knitterMock, setMachineType(Kh270));
  EXPECT_CALL(*arduinoMock, millis);
  expected_write_onPacketReceived(buffer, sizeof(buffer), false);

  // test expectations without destroying instance
  ASSERT_TRUE(Mock::VerifyAndClear(fsmMock));
  ASSERT_TRUE(Mock::VerifyAndClear(knitterMock));
}

TEST_F(ComTest, test_reqstart_fail1) {
  // checksum wrong
  uint8_t buffer[] = {reqStart_msgid, 0, 0, 10, 1, 0x73};
  EXPECT_CALL(*knitterMock, startKnitting).Times(0);
  expected_write_onPacketReceived(buffer, sizeof(buffer), true);

  // test expectations without destroying instance
  ASSERT_TRUE(Mock::VerifyAndClear(knitterMock));
}

TEST_F(ComTest, test_reqstart_fail2) {
  // not enough bytes
  uint8_t buffer[] = {reqStart_msgid, 0, 0, 10, 1, 0x74};
  EXPECT_CALL(*knitterMock, startKnitting).Times(0);
  expected_write_onPacketReceived(buffer, sizeof(buffer) - 1, true);

  // test expectations without destroying instance
  ASSERT_TRUE(Mock::VerifyAndClear(knitterMock));
}

TEST_F(ComTest, test_reqstart_success_KH910) {
  uint8_t buffer[] = {reqStart_msgid, 0, 0, 10, 1, 0x74};
  EXPECT_CALL(*knitterMock, startKnitting);
  expected_write_onPacketReceived(buffer, sizeof(buffer), false);

  // test expectations without destroying instance
  ASSERT_TRUE(Mock::VerifyAndClear(knitterMock));
}

TEST_F(ComTest, test_reqstart_success_KH270) {
  uint8_t buffer[] = {reqStart_msgid, 2, 0, 10, 1, 0x73};
  EXPECT_CALL(*knitterMock, startKnitting);
  expected_write_onPacketReceived(buffer, sizeof(buffer), false);

  // test expectations without destroying instance
  ASSERT_TRUE(Mock::VerifyAndClear(knitterMock));
}

TEST_F(ComTest, test_reqinfo) {
  uint8_t buffer[] = {reqInfo_msgid};
  expected_write_onPacketReceived(buffer, sizeof(buffer), true);
}

TEST_F(ComTest, test_helpCmd) {
  uint8_t buffer[] = {helpCmd_msgid};
  expected_write_onPacketReceived(buffer, sizeof(buffer), false);
}

TEST_F(ComTest, test_sendCmd) {
  uint8_t buffer[] = {sendCmd_msgid};
  expected_write_onPacketReceived(buffer, sizeof(buffer), false);
}

TEST_F(ComTest, test_beepCmd) {
  uint8_t buffer[] = {beepCmd_msgid};
  EXPECT_CALL(*arduinoMock, analogWrite(PIEZO_PIN, _)).Times(AtLeast(1));
  EXPECT_CALL(*arduinoMock, delay(50)).Times(AtLeast(1));
  expected_write_onPacketReceived(buffer, sizeof(buffer), true);
}

TEST_F(ComTest, test_setSingleCmd) {
  uint8_t buffer[] = {setSingleCmd_msgid, 0, 0};
  expected_write_onPacketReceived(buffer, sizeof(buffer), true);
}

TEST_F(ComTest, test_setAllCmd) {
  uint8_t buffer[] = {setAllCmd_msgid, 0, 0};
  expected_write_onPacketReceived(buffer, sizeof(buffer), true);
}

TEST_F(ComTest, test_readEOLsensorsCmd) {
  uint8_t buffer[] = {readEOLsensorsCmd_msgid};
  EXPECT_CALL(*arduinoMock, analogRead(EOL_PIN_L));
  EXPECT_CALL(*arduinoMock, analogRead(EOL_PIN_R));
  expected_write_onPacketReceived(buffer, sizeof(buffer), false);
}

TEST_F(ComTest, test_readEncodersCmd) {
  uint8_t buffer[] = {readEncodersCmd_msgid};
  EXPECT_CALL(*arduinoMock, digitalRead(ENC_PIN_A));
  EXPECT_CALL(*arduinoMock, digitalRead(ENC_PIN_B));
  EXPECT_CALL(*arduinoMock, digitalRead(ENC_PIN_C));
  expected_write_onPacketReceived(buffer, sizeof(buffer), false);
}

TEST_F(ComTest, test_autoReadCmd) {
  uint8_t buffer[] = {autoReadCmd_msgid};
  expected_write_onPacketReceived(buffer, sizeof(buffer), true);
}

TEST_F(ComTest, test_autoTestCmd) {
  uint8_t buffer[] = {autoTestCmd_msgid};
  expected_write_onPacketReceived(buffer, sizeof(buffer), true);
}

TEST_F(ComTest, test_stopCmd) {
  uint8_t buffer[] = {stopCmd_msgid};
  com->onPacketReceived(buffer, sizeof(buffer));
}

TEST_F(ComTest, test_quitCmd) {
  uint8_t buffer[] = {quitCmd_msgid};
  EXPECT_CALL(*knitterMock, setUpInterrupt);
  EXPECT_CALL(*fsmMock, setState(s_init));
  com->onPacketReceived(buffer, sizeof(buffer));

  // test expectations without destroying instance
  ASSERT_TRUE(Mock::VerifyAndClear(knitterMock));
  ASSERT_TRUE(Mock::VerifyAndClear(fsmMock));
}

TEST_F(ComTest, test_unrecognized) {
  uint8_t buffer[] = {0xFF};
  com->onPacketReceived(buffer, sizeof(buffer));
}

TEST_F(ComTest, test_cnfline_kh910) {
  // dummy pattern
  uint8_t pattern[] = {1};

  // message for machine with 200 needles
  uint8_t buffer[30] = {cnfLine_msgid /* 0x42 */,
                        0,
                        0,
                        1,
                        0xDE,
                        0xAD,
                        0xBE,
                        0xEF,
                        0x00,
                        0x00,
                        0x00,
                        0x00,
                        0x00,
                        0x00,
                        0x00,
                        0x00,
                        0x00,
                        0x00,
                        0x00,
                        0x00,
                        0x00,
                        0x00,
                        0x00,
                        0x00,
                        0x00,
                        0x00,
                        0x00,
                        0x00,
                        0x00,
                        0xA7}; // CRC8

  // start KH910 job
  knitterMock->startKnitting(Kh910, 0, 199, pattern, false);

  // first call increments line number to zero, not accepted
  EXPECT_CALL(*knitterMock, setNextLine).WillOnce(Return(false));
  EXPECT_CALL(*knitterMock, setLastLine).Times(0);
  com->onPacketReceived(buffer, sizeof(buffer));

  // second call Line accepted, last line
  EXPECT_CALL(*knitterMock, setNextLine).WillOnce(Return(true));
  EXPECT_CALL(*knitterMock, setLastLine).Times(1);
  com->onPacketReceived(buffer, sizeof(buffer));

  // not last line
  buffer[3] = 0x00;
  buffer[29] = 0xC0;
  EXPECT_CALL(*knitterMock, setNextLine).WillOnce(Return(true));
  EXPECT_CALL(*knitterMock, setLastLine).Times(0);
  com->onPacketReceived(buffer, sizeof(buffer));

  // checksum wrong
  EXPECT_CALL(*knitterMock, setNextLine).Times(0);
  buffer[29]--;
  com->onPacketReceived(buffer, sizeof(buffer));

  // not enough bytes in buffer
  EXPECT_CALL(*knitterMock, setNextLine).Times(0);
  com->onPacketReceived(buffer, sizeof(buffer) - 1);

  // test expectations without destroying instance
  ASSERT_TRUE(Mock::VerifyAndClear(knitterMock));
}

/*
TEST_F(ComTest, test_cnfline_kh270) {
  // dummy pattern
  uint8_t pattern[] = {1};

  // message for KH270
  // CRC8 calculated with
  // http://tomeko.net/online_tools/crc8.php?lang=en
  uint8_t buffer[20] = {cnfLine_msgid, 0, 0, 1,
                        0xDE, 0xAD, 0xBE, 0xEF, 0x00,
                        0x00, 0x00, 0x00, 0x00, 0x00,
                        0x00, 0x00, 0x00, 0x00, 0x00,
                        0xab};  // CRC8
  // start KH270 job
  knitterMock->startOperation(Kh270, 0, 113, pattern, false);
  com->onPacketReceived(buffer, sizeof(buffer));
}
*/

TEST_F(ComTest, test_debug) {
  uint8_t buffer[] = {debug_msgid};
  com->onPacketReceived(buffer, sizeof(buffer));
}

TEST_F(ComTest, test_update) {
  EXPECT_CALL(*serialMock, available);
  com->update();
}

TEST_F(ComTest, test_send) {
  expect_write(true);
  uint8_t p[] = {1, 2, 3};
  com->send(p, 3);
}

TEST_F(ComTest, test_sendMsg1) {
  expect_write(true);
  com->sendMsg(testRes_msgid, "abc");
}

TEST_F(ComTest, test_sendMsg2) {
  char buf[] = "abc\0";
  expect_write(true);
  com->sendMsg(testRes_msgid, buf);
}

TEST_F(ComTest, test_send_reqLine) {
  expect_write(true);
  com->send_reqLine(0);
}

TEST_F(ComTest, test_send_indState) {
  EXPECT_CALL(*arduinoMock, analogRead(EOL_PIN_L));
  EXPECT_CALL(*arduinoMock, analogRead(EOL_PIN_R));
  expect_write(true);
  com->send_indState(Knit, 0, true);
}
