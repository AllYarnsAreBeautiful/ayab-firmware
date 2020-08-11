/*!`
 * \file test_knitter.cpp
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

#include <beeper_mock.h>
#include <board.h>
#include <encoders_mock.h>
#include <hw_test_mock.h>
#include <knitter.h>
#include <serial_encoding_mock.h>
#include <solenoids_mock.h>

using ::testing::_;
using ::testing::AtLeast;
using ::testing::Return;

// global definitions
// references everywhere else must use `extern`
Knitter *knitter;
HardwareTest *hwTest;

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
    hwTestMock = hardwareTestMockInstance();
    expect_constructor();
    k = new Knitter();
  }

  void TearDown() override {
    releaseArduinoMock();
    releaseBeeperMock();
    releaseSolenoidsMock();
    releaseEncodersMock();
    releaseSerialEncodingMock();
    releaseHardwareTestMock();
  }

  ArduinoMock *arduinoMock;
  BeeperMock *beeperMock;
  SolenoidsMock *solenoidsMock;
  EncodersMock *encodersMock;
  SerialEncodingMock *serialEncodingMock;
  HardwareTestMock *hwTestMock;
  Knitter *k;

  void expect_constructor() {
    EXPECT_CALL(*arduinoMock, pinMode(ENC_PIN_A, INPUT));
    EXPECT_CALL(*arduinoMock, pinMode(ENC_PIN_B, INPUT));
    EXPECT_CALL(*arduinoMock, pinMode(ENC_PIN_C, INPUT));

    EXPECT_CALL(*arduinoMock, pinMode(LED_PIN_A, OUTPUT));
    EXPECT_CALL(*arduinoMock, pinMode(LED_PIN_B, OUTPUT));

    EXPECT_CALL(*arduinoMock, digitalWrite(LED_PIN_A, 1)); // green LED on
    EXPECT_CALL(*arduinoMock, digitalWrite(LED_PIN_B, 1)); // yellow LED on

    EXPECT_CALL(*solenoidsMock, init);
  }

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
    expect_isr(1, dir, hall, Regular, Garter);
  }

  void expected_isr(Direction_t dir, Direction_t hall) {
    expect_isr(dir, hall);
    k->isr();
  }

  void expect_isr(uint16_t pos) {
    expect_isr(pos, Right, Left, Regular, Garter);
  }

  void expected_isr(uint16_t pos) {
    expect_isr(pos);
    k->isr();
  }

  void expected_isr() {
    expected_isr(1);
  }

  template <int times = 1> void expect_send() {
    EXPECT_CALL(*serialEncodingMock, send).Times(times);
  }

  template <int times = 1> void expect_indState() {
    expect_send<times>();
    EXPECT_CALL(*encodersMock, getHallValue(Left)).Times(times);
    EXPECT_CALL(*encodersMock, getHallValue(Right)).Times(times);
    EXPECT_CALL(*encodersMock, getDirection).Times(times);
  }

  void expect_fsm() {
    EXPECT_CALL(*serialEncodingMock, update);
  }

  void expected_fsm() {
    expect_fsm();
    k->fsm();
  }

  void get_to_ready() {
    // Machine is initialized when left hall sensor is passed in Right direction
    // Inside active needles
    Machine_t m = k->getMachineType();
    expected_isr(40 + END_OF_LINE_OFFSET_L[m] + 1);

    // init
    EXPECT_CALL(*solenoidsMock, setSolenoids(0xFFFF));
    expect_indState();
    expected_fsm();
  }

  void get_to_operate(Machine_t m) {
    get_to_ready();
    uint8_t pattern[] = {1};
    EXPECT_CALL(*beeperMock, ready);
    EXPECT_CALL(*encodersMock, init);
    k->startOperation(m, 0, NUM_NEEDLES[m] - 1, pattern, false);
  }

  void expected_operate(bool first) {
    if (first) {
      get_to_operate(Kh910);
      EXPECT_CALL(*arduinoMock, delay(2000));
      EXPECT_CALL(*beeperMock, finishedLine);
      expect_send();
    }
    EXPECT_CALL(*arduinoMock, digitalWrite(LED_PIN_A, 1)); // green LED on
    expected_fsm();
  }

  void expected_test(bool first) {
    if (first) {
      // FIXME(TP): this is a kludge -
      // currently there is no way to set the machineType in test mode
      k->setMachineType(Kh910);
      EXPECT_CALL(*encodersMock, init);
      encodersMock->init(Kh910);
      ASSERT_EQ(k->startTest(), true);
    }
    expect_indState();
    expected_fsm();
  }

  void test_operate_line_request() {
    EXPECT_CALL(*solenoidsMock, setSolenoid);
    expected_operate(true);
    // _workedOnLine is set to true

    // Position has changed since last call to operate function
    // m_pixelToSet is set above m_stopNeedle + END_OF_LINE_OFFSET_R
    Machine_t m = k->getMachineType(); // Kh910
    expected_isr(NUM_NEEDLES[m] + 8 + END_OF_LINE_OFFSET_R[m] + 1);

    EXPECT_CALL(*solenoidsMock, setSolenoid);
    expected_operate(false);

    // No change in position, no action.
    EXPECT_CALL(*solenoidsMock, setSolenoid).Times(0);
    expected_operate(false);
  }
};

/*!
 * \test
 */
TEST_F(KnitterTest, test_constructor) {
  ASSERT_EQ(k->getState(), s_init);
  // NOTE: Probing private data!
  ASSERT_EQ(k->getStartNeedle(), 0);
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
  k->setState(static_cast<OpState_t>(4));
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
TEST_F(KnitterTest, test_startOperation_NoMachine) {
  uint8_t pattern[] = {1};
  ASSERT_EQ(k->getState(), s_init);
  Machine_t m = k->getMachineType();
  ASSERT_EQ(m, NoMachine);
  get_to_ready();
  ASSERT_EQ(k->startOperation(m, 0, NUM_NEEDLES[m] - 1, pattern, false), false);
}

/*!
 * \test
 */
TEST_F(KnitterTest, test_startOperation_notReady) {
  uint8_t pattern[] = {1};
  ASSERT_EQ(k->getState(), s_init);
  ASSERT_EQ(k->startOperation(Kh910, 0, NUM_NEEDLES[Kh910] - 1, pattern, false),
            false);
}

/*!
 * \test
 */
TEST_F(KnitterTest, test_startOperation_Kh910) {
  uint8_t pattern[] = {1};
  get_to_ready();
  EXPECT_CALL(*encodersMock, init);
  EXPECT_CALL(*beeperMock, ready);
  ASSERT_EQ(k->startOperation(Kh910, 0, NUM_NEEDLES[Kh910] - 1, pattern, false),
            true);
}

/*!
 * \test
 */
TEST_F(KnitterTest, test_startOperation_Kh270) {
  uint8_t pattern[] = {1};
  get_to_ready();
  EXPECT_CALL(*encodersMock, init);
  EXPECT_CALL(*beeperMock, ready);
  ASSERT_EQ(k->startOperation(Kh270, 0, NUM_NEEDLES[Kh270] - 1, pattern, false),
            true);
}

/*!
 * \test
 */
TEST_F(KnitterTest, test_startOperation_failures) {
  uint8_t pattern[] = {1};

  // stopNeedle lower than start
  get_to_ready();
  ASSERT_EQ(k->startOperation(Kh910, 1, 0, pattern, false), false);
  ASSERT_EQ(k->startOperation(Kh910, 0, NUM_NEEDLES[Kh910], pattern, false),
            false);
  ASSERT_EQ(k->startOperation(Kh910, 0, NUM_NEEDLES[Kh910] - 1, nullptr, false),
            false);
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
  get_to_operate(Kh910);
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
  expected_isr(40 + NUM_NEEDLES[Kh910] - 1 + END_OF_LINE_OFFSET_R[Kh910] + 1);
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
TEST_F(KnitterTest, test_operate_Kh910) {
  // m_pixelToSet gets set to 0
  expected_isr(8);

  // init
  uint16_t bitmask = SOLENOIDS_BITMASK;
  EXPECT_CALL(*solenoidsMock, setSolenoids(bitmask));
  expect_indState();
  expected_fsm();

  // operate
  uint8_t pattern[] = {1};

  // startNeedle is greater than pixelToSet
  EXPECT_CALL(*beeperMock, ready);
  EXPECT_CALL(*encodersMock, init);
  const uint8_t START_NEEDLE = NUM_NEEDLES[Kh910] - 2;
  const uint8_t STOP_NEEDLE = NUM_NEEDLES[Kh910] - 1;
  const uint8_t OFFSET = END_OF_LINE_OFFSET_R[Kh910];
  k->startOperation(Kh910, START_NEEDLE, STOP_NEEDLE, pattern, true);

  // first operate
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
  expected_isr(100, NoDirection, Right, Shifted, Garter);
  expect_indState();
  expected_operate(false);

  // Don't set workedonline to true
  expected_isr(8 + STOP_NEEDLE + OFFSET);
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
TEST_F(KnitterTest, test_operate_Kh270) {
  // m_pixelToSet gets set to 0
  expected_isr(8);

  // init
  uint16_t bitmask = SOLENOIDS_BITMASK;
  EXPECT_CALL(*solenoidsMock, setSolenoids(bitmask));
  expect_indState();
  expected_fsm();

  // operate
  uint8_t pattern[] = {1};

  // startNeedle is greater than pixelToSet
  EXPECT_CALL(*beeperMock, ready);
  EXPECT_CALL(*encodersMock, init);
  const uint8_t START_NEEDLE = NUM_NEEDLES[Kh270] - 2;
  const uint8_t STOP_NEEDLE = NUM_NEEDLES[Kh270] - 1;
  const uint8_t OFFSET = END_OF_LINE_OFFSET_R[Kh270];
  k->startOperation(Kh270, START_NEEDLE, STOP_NEEDLE, pattern, true);

  // first operate
  EXPECT_CALL(*arduinoMock, digitalWrite(LED_PIN_B, 0))
      .Times(AtLeast(0)); // yellow LED off
  EXPECT_CALL(*arduinoMock, digitalWrite(LED_PIN_B, 1))
      .Times(AtLeast(1)); // yellow LED on
  EXPECT_CALL(*arduinoMock, digitalWrite(LED_PIN_B, 0))
      .Times(AtLeast(0)); // yellow LED off
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
  expected_isr(100, NoDirection, Right, Shifted, Garter);
  expect_indState();
  expected_operate(false);

  // Don't set workedonline to true
  expected_isr(8 + STOP_NEEDLE + OFFSET);
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
  expected_isr(NUM_NEEDLES[Kh910] + 8 + END_OF_LINE_OFFSET_R[Kh910] + 1);

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
  expected_isr(NUM_NEEDLES[Kh910] + 8 + END_OF_LINE_OFFSET_R[Kh910] + 1);

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
  get_to_operate(Kh910);

  // Note probing lots of private data and methods to get full branch coverage.
  k->setStopNeedle(100);
  uint8_t wanted_pixel = k->getStopNeedle() + END_OF_LINE_OFFSET_R[Kh910] + 1;
  k->setFirstRun(false);
  k->setDirection(Left);
  k->setPosition(wanted_pixel + k->getStartOffset(Right));
  k->setWorkedOnLine(true);
  k->setLineRequested(false);
  k->setLastLineFlag(true);

  EXPECT_CALL(*arduinoMock, digitalWrite(LED_PIN_A, 1));
  EXPECT_CALL(*solenoidsMock, setSolenoid);
  EXPECT_CALL(*beeperMock, endWork);
  EXPECT_CALL(*solenoidsMock, setSolenoids(0xFFFF));
  EXPECT_CALL(*beeperMock, finishedLine);
  k->state_operate();

  ASSERT_EQ(k->getStartOffset(NUM_DIRECTIONS), 0);
  k->setCarriage(NUM_CARRIAGES);
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
  expected_isr(NUM_NEEDLES[Kh910] + 8 + END_OF_LINE_OFFSET_R[Kh910] + 1);

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
  expected_isr(100, Right, Right, Shifted, Lace);
  expected_test(true);

  // No direction, need to change position to enter test
  expected_isr(101, NoDirection, Right, Shifted, Lace);
  expected_test(false);

  // No belt, need to change position to enter test
  expected_isr(100, Right, Right, Unknown, Lace);
  expected_test(false);

  // No belt on left side, need to change position to enter test
  expected_isr(101, Left, Right, Unknown, Garter);
  expected_test(false);

  // Left Lace carriage
  expected_isr(100, Left, Right, Unknown, Lace);
  expected_test(false);

  // Regular belt on left, need to change position to enter test
  expected_isr(101, Left, Right, Regular, Garter);
  expected_test(false);

  // Shifted belt on left, need to change position to enter test
  expected_isr(100, Left, Right, Shifted, Garter);
  expected_test(false);

  // Off of right end, position is changed
  expected_isr(END_RIGHT[Kh910], Left, Right, Unknown, Lace);
  expected_test(false);

  // Direction right, have not reached offset
  expected_isr(39, Right, Left, Unknown, Lace);
  expected_test(false);

  // Kh270
  k->setMachineType(Kh270);
  EXPECT_CALL(*encodersMock, init);
  encodersMock->init(Kh270);

  // K carriage direction left
  expected_isr(0, Left, Right, Regular, Knit);
  expected_test(false);

  // K carriage direction right
  expected_isr(END_RIGHT[Kh270], Right, Left, Regular, Knit);
  expected_test(false);
}

/*!
 * \test
 */
TEST_F(KnitterTest, test_getStartOffset) {
  // out of range values
  k->setCarriage(Knit);
  ASSERT_EQ(k->getStartOffset(NoDirection), 0);
  ASSERT_EQ(k->getStartOffset(NUM_DIRECTIONS), 0);
  k->setCarriage(NoCarriage);
  ASSERT_EQ(k->getStartOffset(Left), 0);
  k->setCarriage(NUM_CARRIAGES);
  ASSERT_EQ(k->getStartOffset(Right), 0);
  k->setCarriage(Lace);
  k->setMachineType(NoMachine);
  ASSERT_EQ(k->getStartOffset(Left), 0);
  k->setMachineType(NUM_MACHINES);
  ASSERT_EQ(k->getStartOffset(Right), 0);
}

/*!
 * \test
 */
TEST_F(KnitterTest, test_onPacketReceived) {
  EXPECT_CALL(*serialEncodingMock, onPacketReceived);
  k->onPacketReceived(nullptr, 0);
}
