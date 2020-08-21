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

#include <knitter_mock.h>

using ::testing::_;
using ::testing::Mock;
using ::testing::Return;

extern Com *com;
extern KnitterMock *knitter;

class ComTest : public ::testing::Test {
protected:
  void SetUp() override {
    serialMock = serialMockInstance();

    // pointer to global instance
    knitterMock = knitter;

    // The global instance does not get destroyed at the end of each test.
    // Ordinarily the mock instance would be local and such behaviour would
    // cause a memory leak. We must notify the test that this is not the case.
    Mock::AllowLeak(knitterMock);

    expect_init();
    com->init();
  }

  void TearDown() override {
    releaseKnitterMock();
    releaseSerialMock();
  }

  KnitterMock *knitterMock;
  SerialMock *serialMock;

  void expect_init() {
    EXPECT_CALL(*serialMock, begin);
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
  EXPECT_CALL(*knitterMock, startTest).Times(0);
  com->onPacketReceived(buffer, sizeof(buffer));
  ASSERT_TRUE(Mock::VerifyAndClear(knitterMock));
}

TEST_F(ComTest, test_reqtest_success) {
  uint8_t buffer[] = {reqTest_msgid, Kh270};
  EXPECT_CALL(*knitterMock, startTest).WillOnce(Return(false));
  com->onPacketReceived(buffer, sizeof(buffer));
  ASSERT_TRUE(Mock::VerifyAndClear(knitterMock));
}

TEST_F(ComTest, test_reqstart_fail1) {
  // checksum wrong
  uint8_t buffer[] = {reqStart_msgid, 0, 0, 10, 1, 0x73};
  EXPECT_CALL(*knitterMock, startOperation).Times(0);
  com->onPacketReceived(buffer, sizeof(buffer));
  ASSERT_TRUE(Mock::VerifyAndClear(knitterMock));
}

TEST_F(ComTest, test_reqstart_fail2) {
  // not enough bytes
  uint8_t buffer[] = {reqStart_msgid, 0, 0, 10, 1, 0x74};
  EXPECT_CALL(*knitterMock, startOperation).Times(0);
  com->onPacketReceived(buffer, sizeof(buffer) - 1);
  ASSERT_TRUE(Mock::VerifyAndClear(knitterMock));
}

TEST_F(ComTest, test_reqstart_success_KH910) {
  uint8_t buffer[] = {reqStart_msgid, 0, 0, 10, 1, 0x74};
  EXPECT_CALL(*knitterMock, startOperation);
  com->onPacketReceived(buffer, sizeof(buffer));
  ASSERT_TRUE(Mock::VerifyAndClear(knitterMock));
}

TEST_F(ComTest, test_reqstart_success_KH270) {
  uint8_t buffer[] = {reqStart_msgid, 2, 0, 10, 1, 0x73};
  EXPECT_CALL(*knitterMock, startOperation);
  com->onPacketReceived(buffer, sizeof(buffer));
  ASSERT_TRUE(Mock::VerifyAndClear(knitterMock));
}

TEST_F(ComTest, test_reqinfo) {
  uint8_t buffer[] = {reqInfo_msgid};
  com->onPacketReceived(buffer, sizeof(buffer));
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
                        0xde,
                        0xad,
                        0xbe,
                        0xef,
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
                        0xa7}; // CRC8

  // start KH910 job
  knitterMock->startOperation(Kh910, 0, 199, pattern, false);

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
  buffer[29] = 0xc0;
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
                        0xde, 0xad, 0xbe, 0xef, 0x00,
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
  EXPECT_CALL(*serialMock, write(_, _));
  EXPECT_CALL(*serialMock, write(SLIP::END));
  uint8_t p[] = {1, 2, 3};
  com->send(p, 3);
}

TEST_F(ComTest, test_sendMsg1) {
  EXPECT_CALL(*serialMock, write(_, _));
  EXPECT_CALL(*serialMock, write(SLIP::END));
  com->sendMsg(test_msgid, "abc");
}

TEST_F(ComTest, test_sendMsg2) {
  char buf[] = "abc\0";
  EXPECT_CALL(*serialMock, write(_, _));
  EXPECT_CALL(*serialMock, write(SLIP::END));
  com->sendMsg(test_msgid, buf);
}
