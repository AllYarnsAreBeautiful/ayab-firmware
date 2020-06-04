#include "gtest/gtest.h"

#include "knitter.h"
#include "knitter/beeper_mock.h"
#include "knitter/encoders_mock.h"
#include "knitter/solenoids_mock.h"

using ::testing::_;
using ::testing::NiceMock;
using ::testing::Return;
// using ::testing::StrictMock;

void onPacketReceived(const uint8_t *buffer, size_t size) {
  (void)buffer;
  (void)size;
}

class KnitterTest : public ::testing::Test {
protected:
  void SetUp() override {
    arduinoMock = arduinoMockInstance();
    beeperMock = beeperMockInstance();
    solenoidsMock = solenoidsMockInstance();
    encodersMock = encodersMockInstance();
    serialMock = serialMockInstance();
    EXPECT_CALL(*solenoidsMock, init).Times(1);
    k = new Knitter();
    ASSERT_EQ(k->getState(), s_init);
  }

  void expect_isr(uint8_t pos) {
    EXPECT_CALL(*encodersMock, encA_interrupt);
    EXPECT_CALL(*encodersMock, getPosition).WillRepeatedly(Return(pos));
    EXPECT_CALL(*encodersMock, getDirection).WillRepeatedly(Return(Right));
    EXPECT_CALL(*encodersMock, getHallActive).WillRepeatedly(Return(Left));
    EXPECT_CALL(*encodersMock, getBeltshift).WillRepeatedly(Return(Regular));
    EXPECT_CALL(*encodersMock, getCarriage).WillRepeatedly(Return(G));
  }

  void expect_indState() {
    EXPECT_CALL(*encodersMock, getHallValue(Left));
    EXPECT_CALL(*encodersMock, getHallValue(Right));
    EXPECT_CALL(*encodersMock, getDirection);
  }

  void get_to_operate() {
    // Machine is initialized when left hall sensor is passed in Right direction
    // Inside active needles
    expect_isr(40 + END_OF_LINE_OFFSET_L + 1);
    k->isr();

    // init
    expect_indState();
    k->fsm();

    // operate
    uint8_t line[] = {1};
    k->startOperation(0, NUM_NEEDLES - 1, false, line);
  }

  void TearDown() override {
    releaseArduinoMock();
    releaseBeeperMock();
    releaseSolenoidsMock();
    releaseEncodersMock();
    releaseSerialMock();
  }

  ArduinoMock *arduinoMock;
  BeeperMock *beeperMock;
  SolenoidsMock *solenoidsMock;
  EncodersMock *encodersMock;
  SerialMock *serialMock;
  Knitter *k;
};

/*!
 * \test
 */
TEST_F(KnitterTest, test_constructor) {
  // NOTE: Probing private data!
  ASSERT_EQ(k->m_startNeedle, 0);
}

/*!
 * \test
 */
TEST_F(KnitterTest, test_isr) {
  EXPECT_CALL(*encodersMock, encA_interrupt).Times(1);
  EXPECT_CALL(*encodersMock, getPosition).Times(1).WillOnce(Return(1));
  EXPECT_CALL(*encodersMock, getDirection).Times(1).WillOnce(Return(Right));
  EXPECT_CALL(*encodersMock, getHallActive).Times(1).WillOnce(Return(Left));
  EXPECT_CALL(*encodersMock, getBeltshift).Times(1).WillOnce(Return(Regular));
  EXPECT_CALL(*encodersMock, getCarriage).Times(1).WillOnce(Return(G));
  k->isr();
}

/*!
 * \test
 */
TEST_F(KnitterTest, test_startOperation) {
  uint8_t line[] = {1};
  k->startOperation(0, NUM_NEEDLES - 1, false, line);

  // stopNeedle lower than start
  k->startOperation(1, 0, false, line);

  // stopNeedle equal to NUM_NEEDLES
  k->startOperation(0, NUM_NEEDLES, false, line);
}

/*!
 * \test
 */
TEST_F(KnitterTest, test_startTest) {
  ASSERT_EQ(k->startTest(), true);

  get_to_operate();
  // Can't start test
  ASSERT_EQ(k->startTest(), false);
}

/*!
 * \test
 */
TEST_F(KnitterTest, test_setNextLine) {
  ASSERT_EQ(k->setNextLine(1), false);
}

/*!
 * \test
 */
TEST_F(KnitterTest, test_setLastLine) {
  k->setLastLine();
}

/*!
 * \test
 */
TEST_F(KnitterTest, test_send) {
  uint8_t p[] = {1, 2, 3, 4, 5};
  EXPECT_CALL(*serialMock, write(_, _));
  k->send(p, 5);
}

/*!
 * \test
 */
TEST_F(KnitterTest, test_fsm_init) {
  // Not ready
  EXPECT_CALL(*encodersMock, getDirection).Times(1).WillOnce(Return(Left));
  EXPECT_CALL(*encodersMock, getHallActive).Times(1).WillOnce(Return(Left));

  k->isr();
  k->fsm();
  ASSERT_EQ(k->getState(), s_init);

  // Not now either
  EXPECT_CALL(*encodersMock, getDirection).Times(1).WillOnce(Return(Right));
  EXPECT_CALL(*encodersMock, getHallActive).Times(1).WillOnce(Return(Right));
  k->isr();
  k->fsm();
  ASSERT_EQ(k->getState(), s_init);
}

/*!
 * \test
 */
TEST_F(KnitterTest, test_operate_line_request) {
  get_to_operate();
  k->fsm();
  // _workedOnLine is set to true

  // Position has changed since last call to operate function
  // m_pixelToSet is above m_stopNeedle + END_OF_LINE_OFFSET_R
  expect_isr(NUM_NEEDLES + 8 + END_OF_LINE_OFFSET_R + 1);
  k->isr();

  k->fsm();
}

/*!
 * \test
 */
TEST_F(KnitterTest, test_fsm) {
  // Machine is initialized when left hall sensor is passed in Right direction
  // Inside active needles
  expect_isr(40 + END_OF_LINE_OFFSET_L + 1);
  k->isr();
  EXPECT_CALL(*solenoidsMock, setSolenoids(0xFFFF)).Times(1);
  expect_indState();

  // init
  k->fsm();

  // ready
  EXPECT_CALL(*arduinoMock, digitalWrite(LED_PIN_A, 0)).Times(1);
  k->fsm();
  ASSERT_EQ(k->getState(), s_ready);

  // operate
  EXPECT_CALL(*arduinoMock, digitalWrite(LED_PIN_A, 1)).Times(1);
  uint8_t line[] = {1};
  k->startOperation(0, NUM_NEEDLES - 1, true, line);
  expect_indState();
  EXPECT_CALL(*solenoidsMock, setSolenoid);
  k->fsm();

  // Outside of the active needles
  expect_isr(40 + NUM_NEEDLES - 1 + END_OF_LINE_OFFSET_R + 1);
  k->isr();
  EXPECT_CALL(*arduinoMock, digitalWrite(LED_PIN_A, 1)).Times(1);
  expect_indState();
  EXPECT_CALL(*solenoidsMock, setSolenoid).Times(1);
  k->fsm();
  ASSERT_EQ(k->getState(), s_operate);

  EXPECT_CALL(*beeperMock, finishedLine).Times(0);
  k->setNextLine(1);
  EXPECT_CALL(*beeperMock, finishedLine).Times(1);
  k->setNextLine(0);

  // Reset static position variable for next test
  expect_isr(40 + NUM_NEEDLES - 1 + END_OF_LINE_OFFSET_R + 0);
  k->isr();
  EXPECT_CALL(*arduinoMock, digitalWrite(LED_PIN_A, 1)).Times(1);
  expect_indState();
  EXPECT_CALL(*solenoidsMock, setSolenoid).Times(1);
  k->fsm();
  ASSERT_EQ(k->getState(), s_operate);
}

/*!
 * \test
 */
TEST_F(KnitterTest, test_operate) {
  // m_pixelToSet gets set to 0
  expect_isr(8);
  k->isr();

  // init
  expect_indState();
  k->fsm();

  // operate
  uint8_t line[] = {1};
  // startNeedle is greater than pixelToSet
  k->startOperation(NUM_NEEDLES - 2, NUM_NEEDLES - 1, false, line);

  k->fsm();
}

/*!
 * \test
 */
TEST_F(KnitterTest, test_fsm_test) {
  EXPECT_CALL(*encodersMock, encA_interrupt).Times(1);
  EXPECT_CALL(*encodersMock, getPosition).Times(1).WillOnce(Return(100));
  EXPECT_CALL(*encodersMock, getDirection).Times(1).WillOnce(Return(Left));
  EXPECT_CALL(*encodersMock, getHallActive).Times(1).WillOnce(Return(Right));
  EXPECT_CALL(*encodersMock, getBeltshift).Times(1).WillOnce(Return(Shifted));
  EXPECT_CALL(*encodersMock, getCarriage).Times(1).WillOnce(Return(L));
  k->isr();
  // test
  ASSERT_EQ(k->startTest(), true);
  ASSERT_EQ(k->getState(), s_test);
  EXPECT_CALL(*encodersMock, getHallValue(Left)).Times(1);
  EXPECT_CALL(*encodersMock, getHallValue(Right)).Times(1);
  EXPECT_CALL(*encodersMock, getDirection).Times(1);
  k->fsm();
  EXPECT_CALL(*encodersMock, encA_interrupt).Times(1);
  EXPECT_CALL(*encodersMock, getPosition).Times(1).WillOnce(Return(100));
  EXPECT_CALL(*encodersMock, getDirection).Times(1).WillOnce(Return(Left));
  EXPECT_CALL(*encodersMock, getHallActive).Times(1).WillOnce(Return(Right));
  EXPECT_CALL(*encodersMock, getBeltshift).Times(1).WillOnce(Return(Shifted));
  EXPECT_CALL(*encodersMock, getCarriage).Times(1).WillOnce(Return(L));
  k->isr();
  // test, but don't call indState
  EXPECT_CALL(*encodersMock, getHallValue(Left)).Times(0);
  EXPECT_CALL(*encodersMock, getHallValue(Right)).Times(0);
  EXPECT_CALL(*encodersMock, getDirection).Times(0);
  k->fsm();
  // handle static variable
  expect_isr(0);
  k->isr();
  expect_indState();
  k->fsm();
}

/*!
 * \test
 */
TEST_F(KnitterTest, test_fsm_test2) {
  EXPECT_CALL(*encodersMock, encA_interrupt).Times(1);
  EXPECT_CALL(*encodersMock, getPosition).Times(1).WillOnce(Return(100));
  EXPECT_CALL(*encodersMock, getDirection).Times(1).WillOnce(Return(Left));
  EXPECT_CALL(*encodersMock, getHallActive).Times(1).WillOnce(Return(Right));
  EXPECT_CALL(*encodersMock, getBeltshift).Times(1).WillOnce(Return(Regular));
  EXPECT_CALL(*encodersMock, getCarriage).Times(1).WillOnce(Return(L));
  k->isr();
  // test
  ASSERT_EQ(k->startTest(), true);
  ASSERT_EQ(k->getState(), s_test);
  expect_indState();
  k->fsm();
  // handle static variable
  expect_isr(0);
  k->isr();
  expect_indState();
  k->fsm();
}

/*!
 * \test
 */
TEST_F(KnitterTest, test_fsm_test3) {
  EXPECT_CALL(*encodersMock, encA_interrupt).Times(1);
  EXPECT_CALL(*encodersMock, getPosition).Times(1).WillOnce(Return(100));
  EXPECT_CALL(*encodersMock, getDirection).Times(1).WillOnce(Return(Right));
  EXPECT_CALL(*encodersMock, getHallActive).Times(1).WillOnce(Return(Right));
  EXPECT_CALL(*encodersMock, getBeltshift).Times(1).WillOnce(Return(Shifted));
  EXPECT_CALL(*encodersMock, getCarriage).Times(1).WillOnce(Return(L));
  k->isr();
  // test
  ASSERT_EQ(k->startTest(), true);
  ASSERT_EQ(k->getState(), s_test);
  expect_indState();
  k->fsm();
  // handle static variable
  expect_isr(0);
  k->isr();
  expect_indState();
  k->fsm();
}

/*!
 * \test
 */
TEST_F(KnitterTest, test_calculatePixelAndSolenoid_nodir) {
  EXPECT_CALL(*encodersMock, encA_interrupt).Times(1);
  EXPECT_CALL(*encodersMock, getPosition).Times(1).WillOnce(Return(100));
  EXPECT_CALL(*encodersMock, getDirection)
      .Times(1)
      .WillOnce(Return(NoDirection));
  EXPECT_CALL(*encodersMock, getHallActive).Times(1).WillOnce(Return(Right));
  EXPECT_CALL(*encodersMock, getBeltshift).Times(1).WillOnce(Return(Shifted));
  EXPECT_CALL(*encodersMock, getCarriage).Times(1).WillOnce(Return(L));
  k->isr();
  // test
  ASSERT_EQ(k->startTest(), true);
  ASSERT_EQ(k->getState(), s_test);
  expect_indState();
  k->fsm();
  // handle static variable
  expect_isr(0);
  k->isr();
  expect_indState();
  k->fsm();
}

/*!
 * \test
 */
TEST_F(KnitterTest, test_calculatePixelAndSolenoid_nobelt) {
  EXPECT_CALL(*encodersMock, encA_interrupt).Times(1);
  EXPECT_CALL(*encodersMock, getPosition).Times(1).WillOnce(Return(100));
  EXPECT_CALL(*encodersMock, getDirection).Times(1).WillOnce(Return(Right));
  EXPECT_CALL(*encodersMock, getHallActive).Times(1).WillOnce(Return(Right));
  EXPECT_CALL(*encodersMock, getBeltshift).Times(1).WillOnce(Return(Unknown));
  EXPECT_CALL(*encodersMock, getCarriage).Times(1).WillOnce(Return(L));
  k->isr();
  // test
  ASSERT_EQ(k->startTest(), true);
  ASSERT_EQ(k->getState(), s_test);
  expect_indState();
  k->fsm();
  // handle static variable
  expect_isr(0);
  k->isr();
  expect_indState();
  k->fsm();
}

/*!
 * \test
 */
TEST_F(KnitterTest, test_calculatePixelAndSolenoid_nobelt_left) {
  EXPECT_CALL(*encodersMock, encA_interrupt).Times(1);
  EXPECT_CALL(*encodersMock, getPosition).Times(1).WillOnce(Return(100));
  EXPECT_CALL(*encodersMock, getDirection).Times(1).WillOnce(Return(Left));
  EXPECT_CALL(*encodersMock, getHallActive).Times(1).WillOnce(Return(Right));
  EXPECT_CALL(*encodersMock, getBeltshift).Times(1).WillOnce(Return(Unknown));
  EXPECT_CALL(*encodersMock, getCarriage).Times(1).WillOnce(Return(G));
  k->isr();
  // test
  ASSERT_EQ(k->startTest(), true);
  ASSERT_EQ(k->getState(), s_test);
  expect_indState();
  k->fsm();
  // handle static variable
  expect_isr(0);
  k->isr();
  expect_indState();
  k->fsm();
}

/*!
 * \test
 */
TEST_F(KnitterTest, test_calculatePixelAndSolenoid_off_right_end) {
  EXPECT_CALL(*encodersMock, encA_interrupt).Times(1);
  EXPECT_CALL(*encodersMock, getPosition)
      .Times(1)
      .WillOnce(Return(END_RIGHT + 16 - 1));
  EXPECT_CALL(*encodersMock, getDirection).Times(1).WillOnce(Return(Left));
  EXPECT_CALL(*encodersMock, getHallActive).Times(1).WillOnce(Return(Right));
  EXPECT_CALL(*encodersMock, getBeltshift).Times(1).WillOnce(Return(Unknown));
  EXPECT_CALL(*encodersMock, getCarriage).Times(1).WillOnce(Return(L));
  k->isr();
  // test
  ASSERT_EQ(k->startTest(), true);
  ASSERT_EQ(k->getState(), s_test);
  expect_indState();
  k->fsm();
  // handle static variable
  expect_isr(0);
  k->isr();
  expect_indState();
  k->fsm();
}

/*!
 * \test
 */
TEST_F(KnitterTest, test_operate_lastline) {
  get_to_operate();
  k->fsm();

  // _workedOnLine is set to true

  // Position has changed since last call to operate function
  // m_pixelToSet is above m_stopNeedle + END_OF_LINE_OFFSET_R
  expect_isr(NUM_NEEDLES + 8 + END_OF_LINE_OFFSET_R + 1);
  k->isr();

  // m_lastLineFlag is true
  k->setLastLine();

  EXPECT_CALL(*solenoidsMock, setSolenoid);
  EXPECT_CALL(*beeperMock, endWork);
  EXPECT_CALL(*solenoidsMock, setSolenoids(0xFFFF));
  EXPECT_CALL(*beeperMock, finishedLine);
  k->fsm();
}

/*!
 * \test
 */
TEST_F(KnitterTest, test_fsm_default_case) {
  // NOTE: Probing private data to be able to cover all branches.
  k->m_opState = (OpState_t)4;
  k->fsm();
}

/*!
 * \test
 */
TEST_F(KnitterTest, test_getStartOffset) {
  // NOTE: Probing private method to be able to cover all branches.
  ASSERT_EQ(k->getStartOffset(NoDirection), 0);
}

/*!
 * \test
 */
TEST_F(KnitterTest, test_operate_same_position) {
  get_to_operate();
  expect_isr(1);
  k->isr();
  k->fsm();
  expect_isr(1);
  k->isr();
  k->fsm();
}
