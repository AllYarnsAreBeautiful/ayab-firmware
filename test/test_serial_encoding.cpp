#include "gtest/gtest.h"

#include "serial_encoding.h"
#include "serial_encoding/knitter_mock.h"

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

TEST_F(SerialEncodingTest, test_testmsg) {
  uint8_t buffer[] = {reqTest_msgid};
  s->onPacketReceived(buffer, sizeof(buffer));
}

TEST_F(SerialEncodingTest, test_startmsg) {
  uint8_t buffer[] = {reqStart_msgid, 0, 0, 0};
  s->onPacketReceived(buffer, sizeof(buffer));

  // Not enough bytes
  s->onPacketReceived(buffer, sizeof(buffer) - 1);
}

TEST_F(SerialEncodingTest, test_infomsg) {
  uint8_t buffer[] = {reqInfo_msgid};
  s->onPacketReceived(buffer, sizeof(buffer));
}

TEST_F(SerialEncodingTest, test_cnfmsg) {
  // CRC calculated with
  // http://tomeko.net/online_tools/crc8.php?lang=en
  constexpr uint8_t crc = 0xE9;

  uint8_t buffer[29] = {cnfLine_msgid, // 0x42
                        0x00,          0x00, 0xde, 0xad, 0xbe, 0xef, 0x00,
                        0x00,          0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                        0x00,          0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                        0x00,          0x00, 0x00, 0x00, 0x00, 0x01, crc};

  // Line not accepted
  EXPECT_CALL(*knitterMock, setNextLine);
  s->onPacketReceived(buffer, sizeof(buffer));

  // Line accepted, last line
  EXPECT_CALL(*knitterMock, setLastLine);
  EXPECT_CALL(*knitterMock, setNextLine).WillOnce(Return(true));
  s->onPacketReceived(buffer, sizeof(buffer));

  // Not last line
  buffer[27] = 0x00;
  buffer[28] = 0xB7;
  EXPECT_CALL(*knitterMock, setNextLine).WillOnce(Return(true));
  s->onPacketReceived(buffer, sizeof(buffer));

  // crc wrong
  EXPECT_CALL(*knitterMock, setNextLine).Times(0);
  buffer[28]--;
  s->onPacketReceived(buffer, sizeof(buffer));

  // Not enough bytes in buffer
  EXPECT_CALL(*knitterMock, setNextLine).Times(0);
  s->onPacketReceived(buffer, sizeof(buffer) - 1);
}

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
