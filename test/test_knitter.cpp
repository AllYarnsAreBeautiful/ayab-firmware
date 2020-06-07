#include <gtest/gtest.h>

#include <board.h>
#include <knitter.h>
#include <knitter/beeper_mock.h>
#include <knitter/encoders_mock.h>
#include <knitter/serial_encoding_mock.h>
#include <knitter/solenoids_mock.h>

using ::testing::_;
using ::testing::Return;

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
    serialEncodingMock = serialEncodingMockInstance();
    expect_constructor();
    k = new Knitter();
  }

  void TearDown() override {
    releaseArduinoMock();
    releaseBeeperMock();
    releaseSolenoidsMock();
    releaseEncodersMock();
    releaseSerialEncodingMock();
  }

  ArduinoMock *arduinoMock;
  BeeperMock *beeperMock;
  SolenoidsMock *solenoidsMock;
  EncodersMock *encodersMock;
  SerialEncodingMock *serialEncodingMock;
  Knitter *k;

  void expect_constructor() {
    EXPECT_CALL(*arduinoMock, pinMode(ENC_PIN_A, INPUT));
    EXPECT_CALL(*arduinoMock, pinMode(ENC_PIN_B, INPUT));
    EXPECT_CALL(*arduinoMock, pinMode(ENC_PIN_C, INPUT));

    EXPECT_CALL(*arduinoMock, pinMode(LED_PIN_A, OUTPUT));
    EXPECT_CALL(*arduinoMock, pinMode(LED_PIN_B, OUTPUT));

    EXPECT_CALL(*arduinoMock, digitalWrite(LED_PIN_A, 1));
    EXPECT_CALL(*arduinoMock, digitalWrite(LED_PIN_B, 1));

    EXPECT_CALL(*solenoidsMock, init);
  }

  // TODO(sl): m_position is only uint8_t, should it be bigger?
  void expect_isr(uint16_t pos, Direction_t dir, Direction_t hall,
                  Beltshift_t belt, Carriage_t carriage) {
    EXPECT_CALL(*encodersMock, encA_interrupt);
    EXPECT_CALL(*encodersMock, getPosition).WillRepeatedly(Return(pos));
    EXPECT_CALL(*encodersMock, getDirection).WillRepeatedly(Return(dir));
    EXPECT_CALL(*encodersMock, getHallActive).WillRepeatedly(Return(hall));
    EXPECT_CALL(*encodersMock, getBeltshift).WillRepeatedly(Return(belt));
    EXPECT_CALL(*encodersMock, getCarriage).WillRepeatedly(Return(carriage));
  }
  void expected_isr(uint16_t pos, Direction_t dir, Direction_t hall,
                    Beltshift_t belt, Carriage_t carriage) {
    expect_isr(pos, dir, hall, belt, carriage);
    k->isr();
  }

  void expect_isr(Direction_t dir, Direction_t hall) {
    expect_isr(1, dir, hall, Regular, G);
  }
  void expected_isr(Direction_t dir, Direction_t hall) {
    expect_isr(dir, hall);
    k->isr();
  }

  void expect_isr(uint16_t pos) {
    expect_isr(pos, Right, Left, Regular, G);
  }

  void expected_isr(uint16_t pos) {
    expect_isr(pos);
    k->isr();
  }

  void expected_isr() {
    expected_isr(1);
  }

  template <int times = 1> void expect_send() {
  }

  template <int times = 1> void expect_indState() {
    expect_send<times>();
    EXPECT_CALL(*encodersMock, getHallValue(Left)).Times(times);
    EXPECT_CALL(*encodersMock, getHallValue(Right)).Times(times);
    EXPECT_CALL(*encodersMock, getDirection).Times(times);
  }

  void expect_fsm() {
  }

  void expected_fsm() {
    expect_fsm();
    k->fsm();
  }

  void get_to_ready() {
    // Machine is initialized when left hall sensor is passed in Right direction
    // Inside active needles
    expected_isr(40 + END_OF_LINE_OFFSET_L + 1);

    // init
    EXPECT_CALL(*solenoidsMock, setSolenoids(0xFFFF));
    expect_indState();
    expected_fsm();
  }

  void get_to_operate() {
    get_to_ready();

    EXPECT_CALL(*beeperMock, ready);
    // operate
    uint8_t line[] = {1};
    k->startOperation(0, NUM_NEEDLES - 1, false, line);
  }

  void expected_operate(bool first) {
    if (first) {
      get_to_operate();
      EXPECT_CALL(*arduinoMock, delay(2000));
      EXPECT_CALL(*beeperMock, finishedLine);
      expect_send();
    }
    EXPECT_CALL(*arduinoMock, digitalWrite(LED_PIN_A, 1));
    expected_fsm();
  }

  void expected_test(bool first) {
    if (first) {
      ASSERT_EQ(k->startTest(), true);
    }
    expect_indState();
    expected_fsm();
  }
};

/*!
 * \test
 */
TEST_F(KnitterTest, test_constructor) {
  ASSERT_EQ(k->getState(), s_init);
  // NOTE: Probing private data!
  ASSERT_EQ(k->m_startNeedle, 0);
}

/*!
 * \test
 *
 * \todo sl: Maybe mock packet serial?
 */
TEST_F(KnitterTest, test_send) {
  uint8_t p[] = {1, 2, 3, 4, 5};
  expect_send();
  k->send(p, 5);
}

/*!
 * \test
 */
TEST_F(KnitterTest, test_isr) {
  expected_isr(1);
}

/*!
 * \test
 */
TEST_F(KnitterTest, test_fsm_default_case) {
  // NOTE: Probing private data to be able to cover all branches.
  k->m_opState = (OpState_t)4;
  expected_fsm();
}

/*!
 * \test
 */
TEST_F(KnitterTest, test_fsm_init) {
  // Not ready
  expected_isr(Left, Left);
  expected_fsm();
  ASSERT_EQ(k->getState(), s_init);

  // Not now either
  expected_isr(Right, Right);
  expected_fsm();
  ASSERT_EQ(k->getState(), s_init);

  // Ready
  expected_isr(Right, Left);
  EXPECT_CALL(*solenoidsMock, setSolenoids(0xFFFF));
  expect_indState();
  expected_fsm();
  ASSERT_EQ(k->getState(), s_ready);
}

/*!
 * \test
 */
TEST_F(KnitterTest, test_fsm_ready) {
  get_to_ready();
  EXPECT_CALL(*arduinoMock, digitalWrite(LED_PIN_A, 0));
  expected_fsm();
  // Still in ready
  ASSERT_EQ(k->getState(), s_ready);

  // Again
  EXPECT_CALL(*arduinoMock, digitalWrite(LED_PIN_A, 0));
  expected_fsm();
  // Still in ready
  ASSERT_EQ(k->getState(), s_ready);
}

/*!
 * \test
 */
TEST_F(KnitterTest, test_fsm_test) {
  // Enter test state
  ASSERT_EQ(k->startTest(), true);

  expected_isr();
  expect_indState();
  expected_fsm();

  // Again with same position, no indState this time.
  expected_isr();
  expect_indState<0>();
  expected_fsm();
}

/*!
 * \test
 */
TEST_F(KnitterTest, test_startOperation) {
  uint8_t line[] = {1};
  // Not in ready state
  ASSERT_EQ(k->startOperation(0, NUM_NEEDLES - 1, false, line), false);

  get_to_ready();
  EXPECT_CALL(*beeperMock, ready);
  ASSERT_EQ(k->startOperation(0, NUM_NEEDLES - 1, false, line), true);

  // stopNeedle lower than start
  ASSERT_EQ(k->startOperation(1, 0, false, line), false);

  // stopNeedle equal to NUM_NEEDLES
  ASSERT_EQ(k->startOperation(0, NUM_NEEDLES, false, line), false);

  // null pointer passed as line
  ASSERT_EQ(k->startOperation(0, NUM_NEEDLES - 1, false, nullptr), false);
}

/*!
 * \test
 */
TEST_F(KnitterTest, test_startTest) {
  ASSERT_EQ(k->startTest(), true);
  ASSERT_EQ(k->getState(), s_test);
}

/*!
 * \test
 */
TEST_F(KnitterTest, test_startTest_in_operation) {
  get_to_operate();
  // Can't start test
  ASSERT_EQ(k->startTest(), false);
  ASSERT_EQ(k->getState(), s_operate);
}

/*!
 * \test
 */
TEST_F(KnitterTest, test_setNextLine) {
  ASSERT_EQ(k->setNextLine(1), false);

  // Set m_lineRequested
  EXPECT_CALL(*solenoidsMock, setSolenoid).Times(1);
  expected_operate(true);

  // Outside of the active needles
  expected_isr(40 + NUM_NEEDLES - 1 + END_OF_LINE_OFFSET_R + 1);
  EXPECT_CALL(*solenoidsMock, setSolenoid).Times(1);
  expected_operate(false);
  ASSERT_EQ(k->getState(), s_operate);

  // Wrong line number
  EXPECT_CALL(*beeperMock, finishedLine).Times(0);
  expect_send();
  ASSERT_EQ(k->setNextLine(1), false);

  // Correct line number
  EXPECT_CALL(*beeperMock, finishedLine).Times(1);
  ASSERT_EQ(k->setNextLine(0), true);

  // m_lineRequested has been set to false
  ASSERT_EQ(k->setNextLine(0), false);
}

/*!
 * \test
 */
TEST_F(KnitterTest, test_operate) {
  // m_pixelToSet gets set to 0
  expected_isr(8);

  // init
  EXPECT_CALL(*solenoidsMock, setSolenoids(0xFFFF));
  expect_indState();
  expected_fsm();

  // operate
  uint8_t line[] = {1};
  // startNeedle is greater than pixelToSet
  EXPECT_CALL(*beeperMock, ready);
  constexpr uint8_t START_NEEDLE = NUM_NEEDLES - 2;
  constexpr uint8_t STOP_NEEDLE = NUM_NEEDLES - 1;
  k->startOperation(START_NEEDLE, STOP_NEEDLE, true, line);

  // First operate
  EXPECT_CALL(*arduinoMock, delay(2000));
  EXPECT_CALL(*beeperMock, finishedLine);

  // indState and send
  expect_send<2>();
  EXPECT_CALL(*encodersMock, getHallValue(Left));
  EXPECT_CALL(*encodersMock, getHallValue(Right));
  EXPECT_CALL(*encodersMock, getDirection);

  EXPECT_CALL(*solenoidsMock, setSolenoid);
  expected_operate(false);

  // No useful position calculated by calculatePixelAndSolenoid
  expected_isr(100, NoDirection, Right, Shifted, G);
  expect_indState();
  expected_operate(false);

  // Don't set workedonline to true
  expected_isr(8 + STOP_NEEDLE + END_OF_LINE_OFFSET_R);
  EXPECT_CALL(*solenoidsMock, setSolenoid);
  expect_indState();
  expected_operate(false);

  expected_isr(START_NEEDLE);
  EXPECT_CALL(*solenoidsMock, setSolenoid);
  expect_indState();
  expected_operate(false);
}

/*!
 * \test
 */
TEST_F(KnitterTest, test_operate_line_request) {
  EXPECT_CALL(*solenoidsMock, setSolenoid);
  expected_operate(true);
  // _workedOnLine is set to true

  // Position has changed since last call to operate function
  // m_pixelToSet is set above m_stopNeedle + END_OF_LINE_OFFSET_R
  expected_isr(NUM_NEEDLES + 8 + END_OF_LINE_OFFSET_R + 1);

  EXPECT_CALL(*solenoidsMock, setSolenoid);
  expected_operate(false);

  // No change in position, no action.
  EXPECT_CALL(*solenoidsMock, setSolenoid).Times(0);
  expected_operate(false);
}

/*!
 * \test
 */
TEST_F(KnitterTest, test_operate_lastline) {
  EXPECT_CALL(*solenoidsMock, setSolenoid);
  expected_operate(true);
  // _workedOnLine is set to true

  // Position has changed since last call to operate function
  // m_pixelToSet is above m_stopNeedle + END_OF_LINE_OFFSET_R
  expected_isr(NUM_NEEDLES + 8 + END_OF_LINE_OFFSET_R + 1);

  // m_lastLineFlag is true
  k->setLastLine();

  EXPECT_CALL(*solenoidsMock, setSolenoid);
  EXPECT_CALL(*beeperMock, endWork);
  EXPECT_CALL(*solenoidsMock, setSolenoids(0xFFFF));
  EXPECT_CALL(*beeperMock, finishedLine);
  expected_operate(false);
}

/*!
 * \test
 */
TEST_F(KnitterTest, test_operate_lastline_and_no_req) {
  // Note probing lots of private data and methods to get full branch coverage.
  k->m_stopNeedle = 100;
  uint8_t wanted_pixel = k->m_stopNeedle + END_OF_LINE_OFFSET_R + 1;
  k->m_firstRun = false;
  k->m_direction = Left;
  k->m_position = wanted_pixel + k->getStartOffset(Right);
  k->m_workedOnLine = true;
  k->m_lineRequested = false;
  k->m_lastLineFlag = true;

  EXPECT_CALL(*arduinoMock, digitalWrite(LED_PIN_A, 1));
  k->state_operate();

  ASSERT_EQ(k->getStartOffset(NUM_DIRECTIONS), 0);
  k->m_carriage = NUM_CARRIAGES;
  ASSERT_EQ(k->getStartOffset(Right), 0);
}

/*!
 * \test
 */
TEST_F(KnitterTest, test_operate_same_position) {
  EXPECT_CALL(*solenoidsMock, setSolenoid);
  expected_operate(true);

  // No call to setSolenoid since position was the same
  EXPECT_CALL(*solenoidsMock, setSolenoid).Times(0);
  expected_operate(false);
}

/*!
 * \test
 */
TEST_F(KnitterTest, test_operate_new_line) {
  EXPECT_CALL(*solenoidsMock, setSolenoid);
  expected_operate(true);
  // _workedOnLine is set to true

  // Position has changed since last call to operate function
  // m_pixelToSet is above m_stopNeedle + END_OF_LINE_OFFSET_R
  expected_isr(NUM_NEEDLES + 8 + END_OF_LINE_OFFSET_R + 1);

  // Set m_lineRequested to false
  EXPECT_CALL(*beeperMock, finishedLine);
  k->setNextLine(0);

  EXPECT_CALL(*solenoidsMock, setSolenoid);

  // reqLine is called which calls send
  expect_send();
  expected_operate(false);
}

/*!
 * \test
 */
TEST_F(KnitterTest, test_calculatePixelAndSolenoid) {
  // New Position, different beltshift and active hall
  expected_isr(100, Right, Right, Shifted, L);
  expected_test(true);

  // No direction, need to change position to enter test
  expected_isr(101, NoDirection, Right, Shifted, L);
  expected_test(false);

  // No belt, need to change position to enter test
  expected_isr(100, Right, Right, Unknown, L);
  expected_test(false);

  // No belt on left side, need to change position to enter test
  expected_isr(101, Left, Right, Unknown, G);
  expected_test(false);

  // Left L carriage
  expected_isr(100, Left, Right, Unknown, L);
  expected_test(false);

  // Regular belt on left, need to change position to enter test
  expected_isr(101, Left, Right, Regular, G);
  expected_test(false);

  // Shifted belt on left, need to change position to enter test
  expected_isr(100, Left, Right, Shifted, G);
  expected_test(false);

  // Off of right end, position is changed
  expected_isr(END_RIGHT, Left, Right, Unknown, L);
  expected_test(false);
}

/*!
 * \test
 */
TEST_F(KnitterTest, test_getStartOffset) {
  // NOTE: Probing private method to be able to cover all branches.
  ASSERT_EQ(k->getStartOffset(NoDirection), 0);
}
