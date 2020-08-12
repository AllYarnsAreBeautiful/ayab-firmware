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

#include <knitter_mock.h>
#include <serial_encoding.h>

using ::testing::_;
using ::testing::Return;

class SerialEncodingTest : public ::testing::Test {
protected:
  void SetUp() override {
    knitterMock = knitterMockInstance();
    serialMock = serialMockInstance();
    EXPECT_CALL(*serialMock, begin);
    s = new SerialEncoding();
  }

  void TearDown() override {
    releaseKnitterMock();
    releaseSerialMock();
  }

  KnitterMock *knitterMock;
  SerialMock *serialMock;
  SerialEncoding *s;
};
/*
TEST_F(SerialEncodingTest, test_API) {
  ASSERT_EQ(API_VERSION, 6);
}
*/
TEST_F(SerialEncodingTest, test_testmsg) {
  uint8_t buffer[] = {reqTest_msgid};
  EXPECT_CALL(*knitterMock, startTest).WillOnce(Return(false));
  s->onPacketReceived(buffer, sizeof(buffer));

  // no machineType
  EXPECT_CALL(*knitterMock, startTest).Times(0);
  s->onPacketReceived(buffer, sizeof(buffer) - 1);
}

TEST_F(SerialEncodingTest, test_startmsg) {
  uint8_t buffer[] = {reqStart_msgid, 0, 0, 10, 1, 0x74};
  EXPECT_CALL(*knitterMock, startOperation);
  s->onPacketReceived(buffer, sizeof(buffer));
  // checksum wrong
  buffer[5] = 0x73;
  EXPECT_CALL(*knitterMock, startOperation).Times(0);
  s->onPacketReceived(buffer, sizeof(buffer));
  // kh270
  buffer[1] = 2;
  EXPECT_CALL(*knitterMock, startOperation);
  s->onPacketReceived(buffer, sizeof(buffer));
  // Not enough bytes
  EXPECT_CALL(*knitterMock, startOperation).Times(0);
  s->onPacketReceived(buffer, sizeof(buffer) - 1);
}

TEST_F(SerialEncodingTest, test_infomsg) {
  uint8_t buffer[] = {reqInfo_msgid};
  s->onPacketReceived(buffer, sizeof(buffer));
}

TEST_F(SerialEncodingTest, test_cnfmsg_kh910) {
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
  EXPECT_CALL(*knitterMock, setLastLine).Times(0);
  EXPECT_CALL(*knitterMock, setNextLine).WillOnce(Return(false));
  s->onPacketReceived(buffer, sizeof(buffer));

  // second call Line accepted, last line
  EXPECT_CALL(*knitterMock, setLastLine).Times(1);
  EXPECT_CALL(*knitterMock, setNextLine).WillOnce(Return(true));
  s->onPacketReceived(buffer, sizeof(buffer));

  // Not last line
  buffer[3] = 0x00;
  buffer[29] = 0xc0;
  EXPECT_CALL(*knitterMock, setLastLine).Times(0);
  EXPECT_CALL(*knitterMock, setNextLine).WillOnce(Return(true));
  s->onPacketReceived(buffer, sizeof(buffer));

  // crc wrong
  EXPECT_CALL(*knitterMock, setNextLine).Times(0);
  buffer[29]--;
  s->onPacketReceived(buffer, sizeof(buffer));

  // Not enough bytes in buffer
  EXPECT_CALL(*knitterMock, setNextLine).Times(0);
  s->onPacketReceived(buffer, sizeof(buffer) - 1);
}
/*
TEST_F(SerialEncodingTest, test_cnfmsg_kh270) {
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
  s->onPacketReceived(buffer, sizeof(buffer));
}
*/
TEST_F(SerialEncodingTest, test_debug) {
  uint8_t buffer[] = {debug_msgid};
  s->onPacketReceived(buffer, sizeof(buffer));
}

TEST_F(SerialEncodingTest, test_constructor) {
}

TEST_F(SerialEncodingTest, test_update) {
  EXPECT_CALL(*serialMock, available);
  s->update();
}

TEST_F(SerialEncodingTest, test_send) {
  EXPECT_CALL(*serialMock, write(_, _));
  EXPECT_CALL(*serialMock, write(SLIP::END));
  uint8_t p[] = {1, 2, 3};
  s->send(p, 3);
}
