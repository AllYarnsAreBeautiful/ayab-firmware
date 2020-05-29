#include "gtest/gtest.h"

#include "../serial_encoding.h"
#include "serial_encoding/knitter_mock.h"

using ::testing::Return;
class SerialEncodingTest : public ::testing::Test {
protected:
  void SetUp() override {
    knitterMock = knitterMockInstance();
  }

  void TearDown() override {
    releaseKnitterMock();
  }

  KnitterMock *knitterMock;
};

TEST_F(SerialEncodingTest, test_testmsg) {
  uint8_t buffer[] = {reqTest_msgid};
  onPacketReceived(buffer, sizeof(buffer));
}

TEST_F(SerialEncodingTest, test_startmsg) {
  uint8_t buffer[] = {reqStart_msgid};
  onPacketReceived(buffer, sizeof(buffer));
}

TEST_F(SerialEncodingTest, test_infomsg) {
  uint8_t buffer[] = {reqInfo_msgid};
  onPacketReceived(buffer, sizeof(buffer));
}

TEST_F(SerialEncodingTest, test_cnfmsg) {
  uint8_t buffer[] = {cnfLine_msgid,
                      0,
                      0,
                      0,
                      0,
                      0,
                      0,
                      0,
                      0,
                      0,
                      0,
                      0,
                      0,
                      0,
                      0,
                      0,
                      0,
                      0,
                      0,
                      0,
                      0,
                      0,
                      0,
                      0,
                      0,
                      0,
                      0,
                      1,
                      0};
  onPacketReceived(buffer, sizeof(buffer));
  EXPECT_CALL(*knitterMock, setNextLine).WillOnce(Return(true));
  onPacketReceived(buffer, sizeof(buffer));
}

TEST_F(SerialEncodingTest, test_debug) {
  uint8_t buffer[] = {debug_msgid};
  onPacketReceived(buffer, sizeof(buffer));
}
