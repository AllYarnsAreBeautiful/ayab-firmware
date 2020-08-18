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
#include <global_hw_test.h>
#include <hw_test_mock.h>
#include <knitter.h>
#include <serial_encoding_mock.h>
#include <solenoids_mock.h>

using ::testing::_;
using ::testing::AtLeast;
using ::testing::Mock;
using ::testing::Return;
using ::testing::TypedEq;

// Global definition.
// References everywhere else must use `extern`
Knitter *knitter;

// GlobalHardwareTest has static methods which cannot be mocked directly.
// These static methods delegate to non-static methods of a global instance
// implementing the HardwareTestInterface class (i.e. either HardwareTest or
// HardwareTestMock). Here we initialize the member of GlobalHardwareTest
// that points to this instance. This is a form of dependency injection.
HardwareTestInterface *GlobalHardwareTest::m_instance = new HardwareTestMock();

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

    // Pointer to global HardwareTestMock instance
    hwTestMock =
        static_cast<HardwareTestMock *>(GlobalHardwareTest::m_instance);

    // The global instance does not get destroyed at the end of each test.
    // Ordinarily the mock instance would be local and such behaviour would
    // cause a memory leak. We must notify the test that this is not the case.
    Mock::AllowLeak(hwTestMock);

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
  HardwareTestMock *hwTestMock; // pointer to global instance
  Knitter *&k = knitter;        // alias of global `knitter`

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
    // machine is initialized when left hall sensor
    // is passed in Right direction inside active needles
    Machine_t m = k->getMachineType();
    expected_isr(40 + END_OF_LINE_OFFSET_L[m] + 1);

    // initialize
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

  void expected_test() {
    expect_indState();
    EXPECT_CALL(*hwTestMock, loop);
    expected_fsm();
    ASSERT_TRUE(Mock::VerifyAndClear(hwTestMock));
  }

  void expected_set_machine(Machine_t machineType) {
    k->setMachineType(machineType);
    EXPECT_CALL(*encodersMock, init);
    encodersMock->init(machineType);
    ASSERT_EQ(k->startTest(machineType), true);
  }

  void test_operate_line_request() {
    EXPECT_CALL(*solenoidsMock, setSolenoid);
    expected_operate(true);
    // `m_workedOnLine` is set to `true`

    // Position has changed since last call to operate function
    // `m_pixelToSet` is set above `m_stopNeedle` + END_OF_LINE_OFFSET_R
    Machine_t m = k->getMachineType(); // Kh910
    expected_isr(NUM_NEEDLES[m] + 8 + END_OF_LINE_OFFSET_R[m] + 1);

    EXPECT_CALL(*solenoidsMock, setSolenoid);
    expected_operate(false);

    // no change in position, no action.
    EXPECT_CALL(*solenoidsMock, setSolenoid).Times(0);
    expected_operate(false);
  }
};

/*!
 * \test
 */
TEST_F(KnitterTest, test_constructor) {
  ASSERT_EQ(k->getState(), s_init);
  // NOTE: probing private data!
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
  k->setState(static_cast<OpState_t>(4));
  expected_fsm();
}

/*!
 * \test
 */
TEST_F(KnitterTest, test_fsm_init) {
  // not ready
  expected_isr(Left, Left);
  expected_fsm();
  ASSERT_EQ(k->getState(), s_init);

  // still not ready
  expected_isr(Right, Right);
  expected_fsm();
  ASSERT_EQ(k->getState(), s_init);

  // ready
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

  // still in ready state
  ASSERT_EQ(k->getState(), s_ready);

  // again
  EXPECT_CALL(*arduinoMock, digitalWrite(LED_PIN_A, 0));
  expected_fsm();

  // still in ready state
  ASSERT_EQ(k->getState(), s_ready);
}

/*!
 * \test
 */
TEST_F(KnitterTest, test_fsm_hwtest) {
  // enter test state
  EXPECT_CALL(*hwTestMock, setUp);
  ASSERT_EQ(k->startTest(Kh910), true);

  expected_isr();
  expect_indState();
  EXPECT_CALL(*hwTestMock, loop);
  expected_fsm();

  // again with same position, no `indState` this time.
  expected_isr();
  expect_indState<0>();
  EXPECT_CALL(*hwTestMock, loop);
  expected_fsm();

  ASSERT_TRUE(Mock::VerifyAndClear(hwTestMock));
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
  get_to_ready();

  // `m_stopNeedle` lower than `m_startNeedle`
  ASSERT_EQ(k->startOperation(Kh910, 1, 0, pattern, false), false);

  // `m_stopNeedle` out of range
  ASSERT_EQ(k->startOperation(Kh910, 0, NUM_NEEDLES[Kh910], pattern, false),
            false);

  // null pattern
  ASSERT_EQ(k->startOperation(Kh910, 0, NUM_NEEDLES[Kh910] - 1, nullptr, false),
            false);
}

/*!
 * \test
 */
TEST_F(KnitterTest, test_startTest) {
  EXPECT_CALL(*hwTestMock, setUp);
  ASSERT_EQ(k->startTest(Kh910), true);
  ASSERT_EQ(k->getState(), s_test);
  ASSERT_TRUE(Mock::VerifyAndClear(hwTestMock));
}

/*!
 * \test
 */
TEST_F(KnitterTest, test_startTest_in_operation) {
  get_to_operate(Kh910);
  ASSERT_EQ(k->getState(), s_operate);

  // can't start test
  ASSERT_EQ(k->startTest(Kh910), false);
}

/*!
 * \test
 */
TEST_F(KnitterTest, test_setNextLine) {
  ASSERT_EQ(k->setNextLine(1), false);

  // set `m_lineRequested`
  EXPECT_CALL(*solenoidsMock, setSolenoid).Times(1);
  expected_operate(true);

  // outside of the active needles
  expected_isr(40 + NUM_NEEDLES[Kh910] - 1 + END_OF_LINE_OFFSET_R[Kh910] + 1);
  EXPECT_CALL(*solenoidsMock, setSolenoid).Times(1);
  expected_operate(false);
  ASSERT_EQ(k->getState(), s_operate);

  // wrong line number
  EXPECT_CALL(*beeperMock, finishedLine).Times(0);
  expect_send();
  ASSERT_EQ(k->setNextLine(1), false);

  // correct line number
  EXPECT_CALL(*beeperMock, finishedLine).Times(1);
  ASSERT_EQ(k->setNextLine(0), true);

  // `m_lineRequested` has been set to `false`
  ASSERT_EQ(k->setNextLine(0), false);
}

/*!
 * \test
 */
TEST_F(KnitterTest, test_operate_Kh910) {
  // `m_pixelToSet` gets set to 0
  expected_isr(8);

  // initialize
  uint16_t bitmask = SOLENOIDS_BITMASK;
  EXPECT_CALL(*solenoidsMock, setSolenoids(bitmask));
  expect_indState();
  expected_fsm();

  // operate
  uint8_t pattern[] = {1};

  // `m_startNeedle` is greater than `m_pixelToSet`
  EXPECT_CALL(*beeperMock, ready);
  EXPECT_CALL(*encodersMock, init);
  const uint8_t START_NEEDLE = NUM_NEEDLES[Kh910] - 2;
  const uint8_t STOP_NEEDLE = NUM_NEEDLES[Kh910] - 1;
  const uint8_t OFFSET = END_OF_LINE_OFFSET_R[Kh910];
  k->startOperation(Kh910, START_NEEDLE, STOP_NEEDLE, pattern, true);

  // first operate
  EXPECT_CALL(*arduinoMock, delay(2000));
  EXPECT_CALL(*beeperMock, finishedLine);

  // `indState` and send
  expect_send<2>();
  EXPECT_CALL(*encodersMock, getHallValue(Left));
  EXPECT_CALL(*encodersMock, getHallValue(Right));
  EXPECT_CALL(*encodersMock, getDirection);
  EXPECT_CALL(*solenoidsMock, setSolenoid);
  expected_operate(false);

  // no useful position calculated by `calculatePixelAndSolenoid()`
  expected_isr(100, NoDirection, Right, Shifted, Garter);
  expect_indState();
  expected_operate(false);

  // don't set `m_workedonline` to `true`
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
  // `m_pixelToSet` gets set to 0
  expected_isr(8);

  // initialize
  uint16_t bitmask = SOLENOIDS_BITMASK;
  EXPECT_CALL(*solenoidsMock, setSolenoids(bitmask));
  expect_indState();
  expected_fsm();

  // operate
  uint8_t pattern[] = {1};

  // `m_startNeedle` is greater than `m_pixelToSet`
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

  // `indState` and send
  expect_send<2>();
  EXPECT_CALL(*encodersMock, getHallValue(Left));
  EXPECT_CALL(*encodersMock, getHallValue(Right));
  EXPECT_CALL(*encodersMock, getDirection);
  EXPECT_CALL(*solenoidsMock, setSolenoid);
  expected_operate(false);

  // no useful position calculated by `calculatePixelAndSolenoid()`
  expected_isr(100, NoDirection, Right, Shifted, Garter);
  expect_indState();
  expected_operate(false);

  // don't set `m_workedonline` to `true`
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

  // `m_workedOnLine` is set to `true`
  expected_operate(true);

  // Position has changed since last call to operate function
  // `m_pixelToSet` is set above `m_stopNeedle` + END_OF_LINE_OFFSET_R
  expected_isr(NUM_NEEDLES[Kh910] + 8 + END_OF_LINE_OFFSET_R[Kh910] + 1);

  EXPECT_CALL(*solenoidsMock, setSolenoid);
  expected_operate(false);

  // no change in position, no action.
  EXPECT_CALL(*solenoidsMock, setSolenoid).Times(0);
  expected_operate(false);
}

/*!
 * \test
 */
TEST_F(KnitterTest, test_operate_lastline) {
  EXPECT_CALL(*solenoidsMock, setSolenoid);

  // `m_workedOnLine` is set to true
  expected_operate(true);

  // Position has changed since last call to operate function
  // `m_pixelToSet` is above `m_stopNeedle` + END_OF_LINE_OFFSET_R
  expected_isr(NUM_NEEDLES[Kh910] + 8 + END_OF_LINE_OFFSET_R[Kh910] + 1);

  // `m_lastLineFlag` is `true`
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

  // Note: probing private data and methods to get full branch coverage.
  k->m_stopNeedle = 100;
  uint8_t wanted_pixel = k->m_stopNeedle + END_OF_LINE_OFFSET_R[Kh910] + 1;
  k->m_firstRun = false;
  k->m_direction = Left;
  k->m_position = wanted_pixel + k->getStartOffset(Right);
  k->m_workedOnLine = true;
  k->m_lineRequested = false;
  k->m_lastLineFlag = true;

  EXPECT_CALL(*arduinoMock, digitalWrite(LED_PIN_A, 1));
  EXPECT_CALL(*solenoidsMock, setSolenoid);
  EXPECT_CALL(*beeperMock, endWork);
  EXPECT_CALL(*solenoidsMock, setSolenoids(0xFFFF));
  EXPECT_CALL(*beeperMock, finishedLine);
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

  // no call to `setSolenoid()` since position was the same
  EXPECT_CALL(*solenoidsMock, setSolenoid).Times(0);
  expected_operate(false);
}

/*!
 * \test
 */
TEST_F(KnitterTest, test_operate_new_line) {
  EXPECT_CALL(*solenoidsMock, setSolenoid);

  // _workedOnLine is set to true
  expected_operate(true);

  // Position has changed since last call to operate function
  // `m_pixelToSet` is above `m_stopNeedle` + END_OF_LINE_OFFSET_R
  expected_isr(NUM_NEEDLES[Kh910] + 8 + END_OF_LINE_OFFSET_R[Kh910] + 1);

  // set `m_lineRequested` to `false`
  EXPECT_CALL(*beeperMock, finishedLine);
  k->setNextLine(0);

  EXPECT_CALL(*solenoidsMock, setSolenoid);

  // `reqLine()` is called which calls `send()`
  expect_send();
  expected_operate(false);
}

/*!
 * \test
 */
TEST_F(KnitterTest, test_calculatePixelAndSolenoid) {
  EXPECT_CALL(*hwTestMock, setUp);
  expected_set_machine(Kh910);

  // new position, different beltshift and active hall
  expected_isr(100, Right, Right, Shifted, Lace);
  expected_test();

  // no direction, need to change position to enter test
  expected_isr(101, NoDirection, Right, Shifted, Lace);
  expected_test();

  // no belt, need to change position to enter test
  expected_isr(100, Right, Right, Unknown, Lace);
  expected_test();

  // no belt on left side, need to change position to enter test
  expected_isr(101, Left, Right, Unknown, Garter);
  expected_test();

  // left Lace carriage
  expected_isr(100, Left, Right, Unknown, Lace);
  expected_test();

  // regular belt on left, need to change position to enter test
  expected_isr(101, Left, Right, Regular, Garter);
  expected_test();

  // shifted belt on left, need to change position to enter test
  expected_isr(100, Left, Right, Shifted, Garter);
  expected_test();

  // off of right end, position is changed
  expected_isr(END_RIGHT[Kh910], Left, Right, Unknown, Lace);
  expected_test();

  // direction right, have not reached offset
  expected_isr(39, Right, Left, Unknown, Lace);
  expected_test();

  // KH270
  k->setMachineType(Kh270);

  // K carriage direction left
  expected_isr(0, Left, Right, Regular, Knit);
  expected_test();

  // K carriage direction right
  expected_isr(END_RIGHT[Kh270], Right, Left, Regular, Knit);
  expected_test();

  // test expectations without destroying instance
  ASSERT_TRUE(Mock::VerifyAndClear(hwTestMock));
}

/*!
 * \test
 */
TEST_F(KnitterTest, test_getStartOffset) {
  // out of range values
  k->m_carriage = Knit;
  ASSERT_EQ(k->getStartOffset(NoDirection), 0);

  ASSERT_EQ(k->getStartOffset(NUM_DIRECTIONS), 0);

  k->m_carriage = NoCarriage;
  ASSERT_EQ(k->getStartOffset(Left), 0);

  k->m_carriage = NUM_CARRIAGES;
  ASSERT_EQ(k->getStartOffset(Right), 0);

  k->m_carriage = Lace;
  k->m_machineType = NoMachine;
  ASSERT_EQ(k->getStartOffset(Left), 0);

  k->m_machineType = NUM_MACHINES;
  ASSERT_EQ(k->getStartOffset(Right), 0);
}

/*!
 * \test
 */
TEST_F(KnitterTest, test_onPacketReceived) {
  EXPECT_CALL(*serialEncodingMock, onPacketReceived);
  k->onPacketReceived(nullptr, 0);
}

/*!
 * \test
 */
TEST_F(KnitterTest, test_quit_hw_test) {
  // get to test state
  EXPECT_CALL(*hwTestMock, setUp);
  ASSERT_EQ(k->startTest(Kh910), true);
  ASSERT_EQ(k->getState(), s_test);

  // quit
  k->setQuitFlag(true);
  EXPECT_CALL(*hwTestMock, loop);
  k->state_test();
  ASSERT_EQ(k->getState(), s_ready);

  // test expectations without destroying instance
  ASSERT_TRUE(Mock::VerifyAndClear(hwTestMock));
}

/*!
 * \test
 */
TEST_F(KnitterTest, test_setSolenoids) {
  EXPECT_CALL(*solenoidsMock, setSolenoids);
  k->setSolenoids(0xFADE);
}

/*!
 * \test
 */
TEST_F(KnitterTest, test_setSolenoid) {
  EXPECT_CALL(*solenoidsMock, setSolenoid);
  k->setSolenoid(15, 1);
}

/*!
 * \test
 */
TEST_F(KnitterTest, test_sendMsg1) {
  EXPECT_CALL(*serialEncodingMock,
              sendMsg(test_msgid, TypedEq<const char *>("abc")));
  k->sendMsg(test_msgid, "abc");
}

/*!
 * \test
 */
TEST_F(KnitterTest, test_sendMsg2) {
  char buf[] = "abc\0";
  EXPECT_CALL(*serialEncodingMock, sendMsg(test_msgid, TypedEq<char *>(buf)));
  k->sendMsg(test_msgid, buf);
}
