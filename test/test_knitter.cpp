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

#include <board.h>
#include <knitter.h>
#include <tester.h>

#include <beeper_mock.h>
#include <com_mock.h>
#include <encoders_mock.h>
#include <solenoids_mock.h>
#include <tester_mock.h>

using ::testing::_;
using ::testing::AtLeast;
using ::testing::Mock;
using ::testing::Return;
using ::testing::TypedEq;

extern Knitter *knitter;
extern BeeperMock *beeper;
extern ComMock *com;
extern TesterMock *tester;

class KnitterTest : public ::testing::Test {
protected:
  void SetUp() override {
    arduinoMock = arduinoMockInstance();
    solenoidsMock = solenoidsMockInstance();
    encodersMock = encodersMockInstance();

    // pointers to global instances
    beeperMock = beeper;
    comMock = com;
    testerMock = tester;

    // The global instances do not get destroyed at the end of each test.
    // Ordinarily the mock instances would be local and such behaviour would
    // cause a memory leak. We must notify the test that this is not the case.
    Mock::AllowLeak(beeperMock);
    Mock::AllowLeak(comMock);
    Mock::AllowLeak(testerMock);

    expect_init();
    knitter->init();
  }

  void TearDown() override {
    releaseArduinoMock();
    releaseSolenoidsMock();
    releaseEncodersMock();
  }

  ArduinoMock *arduinoMock;
  BeeperMock *beeperMock;
  ComMock *comMock;
  EncodersMock *encodersMock;
  SolenoidsMock *solenoidsMock;
  TesterMock *testerMock;

  void expect_init() {
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
    knitter->isr();
  }

  void expect_isr(Direction_t dir, Direction_t hall) {
    expect_isr(1, dir, hall, Regular, Garter);
  }

  void expected_isr(Direction_t dir, Direction_t hall) {
    expect_isr(dir, hall);
    knitter->isr();
  }

  void expect_isr(uint16_t pos) {
    expect_isr(pos, Right, Left, Regular, Garter);
  }

  void expected_isr(uint16_t pos) {
    expect_isr(pos);
    knitter->isr();
  }

  void expected_isr() {
    expected_isr(1);
  }

  template <int times = 1> void expect_send() {
    EXPECT_CALL(*comMock, send).Times(times);
  }

  template <int times = 1> void expect_indState() {
    expect_send<times>();
    EXPECT_CALL(*encodersMock, getHallValue(Left)).Times(times);
    EXPECT_CALL(*encodersMock, getHallValue(Right)).Times(times);
    EXPECT_CALL(*encodersMock, getDirection).Times(times);
  }

  void expect_fsm() {
    EXPECT_CALL(*comMock, update);
  }

  void expected_fsm() {
    expect_fsm();
    knitter->fsm();
  }

  void get_to_ready() {
    // machine is initialized when left hall sensor
    // is passed in Right direction inside active needles
    Machine_t m = knitter->getMachineType();
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
    knitter->startOperation(m, 0, NUM_NEEDLES[m] - 1, pattern, false);
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
    EXPECT_CALL(*testerMock, loop);
    expected_fsm();

    // test expectations without destroying instance
    ASSERT_TRUE(Mock::VerifyAndClear(testerMock));
  }

  void expected_set_machine(Machine_t machineType) {
    knitter->setMachineType(machineType);
    EXPECT_CALL(*encodersMock, init);
    encodersMock->init(machineType);
    ASSERT_EQ(knitter->startTest(machineType), true);
  }

  void test_operate_line_request() {
    EXPECT_CALL(*solenoidsMock, setSolenoid);
    expected_operate(true);
    // `m_workedOnLine` is set to `true`

    // Position has changed since last call to operate function
    // `m_pixelToSet` is set above `m_stopNeedle` + END_OF_LINE_OFFSET_R
    Machine_t m = knitter->getMachineType(); // Kh910
    expected_isr(NUM_NEEDLES[m] + 8 + END_OF_LINE_OFFSET_R[m] + 1);

    EXPECT_CALL(*solenoidsMock, setSolenoid);
    expected_operate(false);

    // no change in position, no action.
    EXPECT_CALL(*solenoidsMock, setSolenoid).Times(0);
    expected_operate(false);
  }
};

TEST_F(KnitterTest, test_init) {
  ASSERT_EQ(knitter->getState(), s_init);
  ASSERT_EQ(knitter->m_startNeedle, 0);
}

TEST_F(KnitterTest, test_send) {
  uint8_t p[] = {1, 2, 3, 4, 5};
  expect_send();
  comMock->send(p, 5);
}

TEST_F(KnitterTest, test_isr) {
  expected_isr(1);
}

TEST_F(KnitterTest, test_setSolenoids) {
  EXPECT_CALL(*solenoidsMock, setSolenoids);
  knitter->setSolenoids(0xFADE);
}

TEST_F(KnitterTest, test_fsm_default_case) {
  knitter->setState(static_cast<OpState_t>(4));
  expected_fsm();
}

TEST_F(KnitterTest, test_fsm_init_LL) {
  // not ready
  expected_isr(Left, Left);
  expected_fsm();
  ASSERT_EQ(knitter->getState(), s_init);
}

TEST_F(KnitterTest, test_fsm_init_RR) {
  // still not ready
  expected_isr(Right, Right);
  expected_fsm();
  ASSERT_EQ(knitter->getState(), s_init);
}

TEST_F(KnitterTest, test_fsm_init_RL) {
  // ready
  expected_isr(Right, Left);
  EXPECT_CALL(*solenoidsMock, setSolenoids(0xFFFF));
  expect_indState();
  expected_fsm();
  ASSERT_EQ(knitter->getState(), s_ready);
}

TEST_F(KnitterTest, test_fsm_ready) {
  get_to_ready();
  EXPECT_CALL(*arduinoMock, digitalWrite(LED_PIN_A, 0));
  expected_fsm();

  // still in ready state
  ASSERT_EQ(knitter->getState(), s_ready);

  // again
  EXPECT_CALL(*arduinoMock, digitalWrite(LED_PIN_A, 0));
  expected_fsm();

  // still in ready state
  ASSERT_EQ(knitter->getState(), s_ready);
}

TEST_F(KnitterTest, test_fsm_test) {
  // enter test state
  EXPECT_CALL(*testerMock, setUp);
  ASSERT_EQ(knitter->startTest(Kh910), true);

  expected_isr();
  expect_indState();
  EXPECT_CALL(*testerMock, loop);
  expected_fsm();

  // again with same position, no `indState` this time.
  expected_isr();
  expect_indState<0>();
  EXPECT_CALL(*testerMock, loop);
  expected_fsm();

  // test expectations without destroying instance
  ASSERT_TRUE(Mock::VerifyAndClear(testerMock));
}

TEST_F(KnitterTest, test_fsm_test_quit) {
  // enter test state
  EXPECT_CALL(*testerMock, setUp);
  ASSERT_EQ(knitter->startTest(Kh910), true);

  // quit
  EXPECT_CALL(*testerMock, getQuitFlag).WillOnce(Return(true));
  expected_fsm();

  // test expectations without destroying instance
  ASSERT_TRUE(Mock::VerifyAndClear(testerMock));
}

TEST_F(KnitterTest, test_startOperation_NoMachine) {
  uint8_t pattern[] = {1};
  ASSERT_EQ(knitter->getState(), s_init);
  Machine_t m = knitter->getMachineType();
  ASSERT_EQ(m, NoMachine);
  get_to_ready();
  ASSERT_EQ(knitter->startOperation(m, 0, NUM_NEEDLES[m] - 1, pattern, false),
            false);
}

TEST_F(KnitterTest, test_startOperation_notReady) {
  uint8_t pattern[] = {1};
  ASSERT_EQ(knitter->getState(), s_init);
  ASSERT_EQ(
      knitter->startOperation(Kh910, 0, NUM_NEEDLES[Kh910] - 1, pattern, false),
      false);
}

TEST_F(KnitterTest, test_startOperation_Kh910) {
  uint8_t pattern[] = {1};
  get_to_ready();
  EXPECT_CALL(*encodersMock, init);
  EXPECT_CALL(*beeperMock, ready);
  ASSERT_EQ(
      knitter->startOperation(Kh910, 0, NUM_NEEDLES[Kh910] - 1, pattern, false),
      true);

  // test expectations without destroying instance
  ASSERT_TRUE(Mock::VerifyAndClear(beeperMock));
}

TEST_F(KnitterTest, test_startOperation_Kh270) {
  uint8_t pattern[] = {1};
  get_to_ready();
  EXPECT_CALL(*encodersMock, init);
  EXPECT_CALL(*beeperMock, ready);
  ASSERT_EQ(
      knitter->startOperation(Kh270, 0, NUM_NEEDLES[Kh270] - 1, pattern, false),
      true);

  // test expectations without destroying instance
  ASSERT_TRUE(Mock::VerifyAndClear(beeperMock));
}

TEST_F(KnitterTest, test_startOperation_failures) {
  uint8_t pattern[] = {1};
  get_to_ready();

  // `m_stopNeedle` lower than `m_startNeedle`
  ASSERT_EQ(knitter->startOperation(Kh910, 1, 0, pattern, false), false);

  // `m_stopNeedle` out of range
  ASSERT_EQ(
      knitter->startOperation(Kh910, 0, NUM_NEEDLES[Kh910], pattern, false),
      false);

  // null pattern
  ASSERT_EQ(
      knitter->startOperation(Kh910, 0, NUM_NEEDLES[Kh910] - 1, nullptr, false),
      false);
}

TEST_F(KnitterTest, test_startTest) {
  EXPECT_CALL(*testerMock, setUp);
  ASSERT_EQ(knitter->startTest(Kh910), true);
  ASSERT_EQ(knitter->getState(), s_test);

  // test expectations without destroying instance
  ASSERT_TRUE(Mock::VerifyAndClear(testerMock));
}

TEST_F(KnitterTest, test_startTest_in_operation) {
  get_to_operate(Kh910);
  ASSERT_EQ(knitter->getState(), s_operate);

  // can't start test
  ASSERT_EQ(knitter->startTest(Kh910), false);
}

TEST_F(KnitterTest, test_setNextLine) {
  ASSERT_EQ(knitter->setNextLine(1), false);

  // set `m_lineRequested`
  EXPECT_CALL(*solenoidsMock, setSolenoid).Times(1);
  expected_operate(true);

  // outside of the active needles
  expected_isr(40 + NUM_NEEDLES[Kh910] - 1 + END_OF_LINE_OFFSET_R[Kh910] + 1);
  EXPECT_CALL(*solenoidsMock, setSolenoid).Times(1);
  expected_operate(false);
  ASSERT_EQ(knitter->getState(), s_operate);

  // wrong line number
  EXPECT_CALL(*beeperMock, finishedLine).Times(0);
  expect_send();
  ASSERT_EQ(knitter->setNextLine(1), false);

  // correct line number
  EXPECT_CALL(*beeperMock, finishedLine).Times(1);
  ASSERT_EQ(knitter->setNextLine(0), true);

  // `m_lineRequested` has been set to `false`
  ASSERT_EQ(knitter->setNextLine(0), false);

  // test expectations without destroying instance
  ASSERT_TRUE(Mock::VerifyAndClear(beeperMock));
}

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
  knitter->startOperation(Kh910, START_NEEDLE, STOP_NEEDLE, pattern, true);

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

  // test expectations without destroying instance
  ASSERT_TRUE(Mock::VerifyAndClear(beeperMock));
}

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
  knitter->startOperation(Kh270, START_NEEDLE, STOP_NEEDLE, pattern, true);

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

  // test expectations without destroying instance
  ASSERT_TRUE(Mock::VerifyAndClear(beeperMock));
}

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

TEST_F(KnitterTest, test_operate_lastline) {
  EXPECT_CALL(*solenoidsMock, setSolenoid);

  // `m_workedOnLine` is set to true
  expected_operate(true);

  // Position has changed since last call to operate function
  // `m_pixelToSet` is above `m_stopNeedle` + END_OF_LINE_OFFSET_R
  expected_isr(NUM_NEEDLES[Kh910] + 8 + END_OF_LINE_OFFSET_R[Kh910] + 1);

  // `m_lastLineFlag` is `true`
  knitter->setLastLine();

  EXPECT_CALL(*solenoidsMock, setSolenoid);
  EXPECT_CALL(*beeperMock, endWork);
  EXPECT_CALL(*solenoidsMock, setSolenoids(0xFFFF));
  EXPECT_CALL(*beeperMock, finishedLine);
  expected_operate(false);

  // test expectations without destroying instance
  ASSERT_TRUE(Mock::VerifyAndClear(beeperMock));
}

TEST_F(KnitterTest, test_operate_lastline_and_no_req) {
  get_to_operate(Kh910);

  // Note: probing private data and methods to get full branch coverage.
  knitter->m_stopNeedle = 100;
  uint8_t wanted_pixel =
      knitter->m_stopNeedle + END_OF_LINE_OFFSET_R[Kh910] + 1;
  knitter->m_firstRun = false;
  knitter->m_direction = Left;
  knitter->m_position = wanted_pixel + knitter->getStartOffset(Right);
  knitter->m_workedOnLine = true;
  knitter->m_lineRequested = false;
  knitter->m_lastLineFlag = true;

  EXPECT_CALL(*arduinoMock, digitalWrite(LED_PIN_A, 1));
  EXPECT_CALL(*solenoidsMock, setSolenoid);
  EXPECT_CALL(*beeperMock, endWork);
  EXPECT_CALL(*solenoidsMock, setSolenoids(0xFFFF));
  EXPECT_CALL(*beeperMock, finishedLine);
  knitter->state_operate();

  ASSERT_EQ(knitter->getStartOffset(NUM_DIRECTIONS), 0);
  knitter->m_carriage = NUM_CARRIAGES;
  ASSERT_EQ(knitter->getStartOffset(Right), 0);

  // test expectations without destroying instance
  ASSERT_TRUE(Mock::VerifyAndClear(beeperMock));
}

TEST_F(KnitterTest, test_operate_same_position) {
  EXPECT_CALL(*solenoidsMock, setSolenoid);
  expected_operate(true);

  // no call to `setSolenoid()` since position was the same
  EXPECT_CALL(*solenoidsMock, setSolenoid).Times(0);
  expected_operate(false);
}

TEST_F(KnitterTest, test_operate_new_line) {
  EXPECT_CALL(*solenoidsMock, setSolenoid);

  // _workedOnLine is set to true
  expected_operate(true);

  // Position has changed since last call to operate function
  // `m_pixelToSet` is above `m_stopNeedle` + END_OF_LINE_OFFSET_R
  expected_isr(NUM_NEEDLES[Kh910] + 8 + END_OF_LINE_OFFSET_R[Kh910] + 1);

  // set `m_lineRequested` to `false`
  EXPECT_CALL(*beeperMock, finishedLine);
  knitter->setNextLine(0);

  EXPECT_CALL(*solenoidsMock, setSolenoid);

  // `reqLine()` is called which calls `send()`
  expect_send();
  expected_operate(false);

  // test expectations without destroying instance
  ASSERT_TRUE(Mock::VerifyAndClear(beeperMock));
}

TEST_F(KnitterTest, test_calculatePixelAndSolenoid) {
  EXPECT_CALL(*testerMock, setUp);
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
  knitter->setMachineType(Kh270);

  // K carriage direction left
  expected_isr(0, Left, Right, Regular, Knit);
  expected_test();

  // K carriage direction right
  expected_isr(END_RIGHT[Kh270], Right, Left, Regular, Knit);
  expected_test();

  // test expectations without destroying instance
  ASSERT_TRUE(Mock::VerifyAndClear(testerMock));
}

TEST_F(KnitterTest, test_getStartOffset) {
  // out of range values
  knitter->m_carriage = Knit;
  ASSERT_EQ(knitter->getStartOffset(NoDirection), 0);

  ASSERT_EQ(knitter->getStartOffset(NUM_DIRECTIONS), 0);

  knitter->m_carriage = NoCarriage;
  ASSERT_EQ(knitter->getStartOffset(Left), 0);

  knitter->m_carriage = NUM_CARRIAGES;
  ASSERT_EQ(knitter->getStartOffset(Right), 0);

  knitter->m_carriage = Lace;
  knitter->m_machineType = NoMachine;
  ASSERT_EQ(knitter->getStartOffset(Left), 0);

  knitter->m_machineType = NUM_MACHINES;
  ASSERT_EQ(knitter->getStartOffset(Right), 0);
}
