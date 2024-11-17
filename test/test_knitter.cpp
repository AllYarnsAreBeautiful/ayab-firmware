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

#include <beeper_mock.h>
#include <com_mock.h>
#include <encoders_mock.h>
#include <fsm_mock.h>
#include <solenoids_mock.h>
#include <tester_mock.h>

using ::testing::_;
using ::testing::AtLeast;
using ::testing::AtMost;
using ::testing::AnyNumber;
using ::testing::Assign;
using ::testing::Mock;
using ::testing::Return;
using ::testing::TypedEq;

extern Knitter *knitter;
extern Fsm *fsm;

extern BeeperMock *beeper;
extern ComMock *com;
extern EncodersMock *encoders;
extern SolenoidsMock *solenoids;
extern TesterMock *tester;

class KnitterTest : public ::testing::Test {
protected:
  void SetUp() override {
    arduinoMock = arduinoMockInstance();

    // pointers to global instances
    beeperMock = beeper;
    comMock = com;
    encodersMock = encoders;
    solenoidsMock = solenoids;
    testerMock = tester;

    // The global instances do not get destroyed at the end of each test.
    // Ordinarily the mock instances would be local and such behaviour would
    // cause a memory leak. We must notify the test that this is not the case.
    Mock::AllowLeak(beeperMock);
    Mock::AllowLeak(comMock);
    Mock::AllowLeak(encodersMock);
    Mock::AllowLeak(solenoidsMock);
    Mock::AllowLeak(testerMock);

    // start in state `OpState::init`
    expected_isr(Direction_t::NoDirection, Direction_t::NoDirection);
    EXPECT_CALL(*arduinoMock, millis);
    fsm->init();
    expect_knitter_init();
    knitter->init();
  }

  void TearDown() override {
    releaseArduinoMock();
  }

  ArduinoMock *arduinoMock;
  BeeperMock *beeperMock;
  ComMock *comMock;
  EncodersMock *encodersMock;
  SolenoidsMock *solenoidsMock;
  TesterMock *testerMock;

  uint8_t get_position_past_left() {
    return (END_LEFT_PLUS_OFFSET[static_cast<uint8_t>(encoders->getMachineType())] + GARTER_SLOP) + 1;
  }

  uint8_t get_position_past_right() {
    return (END_RIGHT_MINUS_OFFSET[static_cast<uint8_t>(encoders->getMachineType())] - GARTER_SLOP) - 1;
  }

  void expect_knitter_init() {
    EXPECT_CALL(*arduinoMock, pinMode(ENC_PIN_A, INPUT));
    EXPECT_CALL(*arduinoMock, pinMode(ENC_PIN_B, INPUT));
    EXPECT_CALL(*arduinoMock, pinMode(ENC_PIN_C, INPUT));

    EXPECT_CALL(*arduinoMock, pinMode(LED_PIN_A, OUTPUT));
    EXPECT_CALL(*arduinoMock, pinMode(LED_PIN_B, OUTPUT));

    EXPECT_CALL(*arduinoMock, digitalWrite(LED_PIN_A, HIGH)); // green LED on
    EXPECT_CALL(*arduinoMock, digitalWrite(LED_PIN_B, HIGH)); // yellow LED on

    EXPECT_CALL(*solenoidsMock, init);
  }

  void expect_isr(uint16_t pos, Direction_t dir, Direction_t hall,
                  BeltShift_t belt, Carriage_t carriage) {
    EXPECT_CALL(*encodersMock, encA_interrupt);
    EXPECT_CALL(*encodersMock, getPosition).WillRepeatedly(Return(pos));
    EXPECT_CALL(*encodersMock, getDirection).WillRepeatedly(Return(dir));
    EXPECT_CALL(*encodersMock, getHallActive).WillRepeatedly(Return(hall));
    EXPECT_CALL(*encodersMock, getBeltShift).WillRepeatedly(Return(belt));
    EXPECT_CALL(*encodersMock, getCarriage).WillRepeatedly(Return(carriage));
  }

  void expected_isr(uint16_t pos, Direction_t dir, Direction_t hall,
                    BeltShift_t belt, Carriage_t carriage) {
    expect_isr(pos, dir, hall, belt, carriage);
    knitter->isr();
  }

  void expect_isr(Direction_t dir, Direction_t hall) {
    expect_isr(1, dir, hall, BeltShift::Regular, Carriage_t::Knit);
  }

  void expected_isr(uint8_t pos, Direction_t dir, Direction_t hall) {
    expect_isr(pos, dir, hall, BeltShift::Regular, Carriage_t::Knit);
    knitter->isr();
  }

  void expected_isr(Direction_t dir, Direction_t hall) {
    expect_isr(dir, hall);
    knitter->isr();
  }

  void expect_isr(uint16_t pos) {
    expect_isr(pos, Direction_t::Right, Direction_t::Left, BeltShift::Regular, Carriage_t::Garter);
  }

  void expected_isr(uint16_t pos) {
    expect_isr(pos);
    knitter->isr();
  }

  void expected_isr() {
    expected_isr(1);
  }

  void expect_reqLine() {
    EXPECT_CALL(*comMock, send_reqLine);
  }

  void expect_indState() {
    EXPECT_CALL(*comMock, send_indState);
  }

  void expected_dispatch() {
    EXPECT_CALL(*comMock, update);
    fsm->dispatch();
  }

  void expected_get_ready() {
    // starts in state `OpState::wait_for_machine`
    ASSERT_EQ(fsm->getState(), OpState::init);

    EXPECT_CALL(*solenoidsMock, setSolenoids(SOLENOIDS_BITMASK));
    expect_indState();
    expected_dispatch_init();

    ASSERT_EQ(fsm->getState(), OpState::ready);
  }

  void expected_init_machine(Machine_t m) {
    // Init the machine
    ASSERT_EQ(knitter->initMachine(m), ErrorCode::success);
    expected_dispatch_wait_for_machine();

    ASSERT_EQ(fsm->getState(), OpState::init);
  }

  void get_to_ready(Machine_t m) {
    expected_init_machine(m);
    // Machine is initialized when Left hall sensor
    // is passed in Right direction inside active needles.
    uint8_t position = get_position_past_left();
    expected_isr(position, Direction_t::Right, Direction_t::Left);
    expected_get_ready();
  }

  void get_to_knit(Machine_t m) {
    EXPECT_CALL(*encodersMock, init);
    get_to_ready(m);
    uint8_t pattern[] = {1};
    EXPECT_CALL(*beeperMock, ready);
    ASSERT_EQ(knitter->startKnitting(0, NUM_NEEDLES[static_cast<uint8_t>(m)] - 1, pattern, false), ErrorCode::success);
    expected_dispatch_ready();

    // ends in state `OpState::knit`
    ASSERT_TRUE(fsm->getState() == OpState::knit);
  }

  void expected_dispatch_knit(bool first) {
    if (first) {
      get_to_knit(Machine_t::Kh910);
      expect_first_knit();
      EXPECT_CALL(*arduinoMock, digitalWrite(LED_PIN_A, HIGH)); // green LED on
      expected_dispatch();
      return;
    }
    ASSERT_TRUE(fsm->getState() == OpState::knit);
    EXPECT_CALL(*arduinoMock, digitalWrite(LED_PIN_A, HIGH)); // green LED on
    expected_dispatch();
  }

  void expected_dispatch_wait_for_machine() {
    // starts in state `OpState::init`
    ASSERT_EQ(fsm->getState(), OpState::wait_for_machine);

    EXPECT_CALL(*arduinoMock, digitalWrite(LED_PIN_A, LOW));
    expected_dispatch();
  }

  void expected_dispatch_init() {
    // starts in state `OpState::init`
    ASSERT_EQ(fsm->getState(), OpState::init);

    EXPECT_CALL(*arduinoMock, digitalWrite(LED_PIN_A, LOW));
    expected_dispatch();
  }

  void expected_dispatch_ready() {
    // starts in state `OpState::ready`
    ASSERT_TRUE(fsm->getState() == OpState::ready);

    EXPECT_CALL(*arduinoMock, digitalWrite(LED_PIN_A, LOW));
    expected_dispatch();
  }

  void expected_dispatch_test() {
    // starts in state `OpState::test`
    ASSERT_EQ(fsm->getState(), OpState::test);

    expect_indState();
    EXPECT_CALL(*testerMock, loop);
    expected_dispatch();
  }

  void expect_first_knit() {
    EXPECT_CALL(*beeperMock, finishedLine);
    expect_reqLine();
  }
};

TEST_F(KnitterTest, test_send) {
  uint8_t p[] = {1, 2, 3, 4, 5};
  EXPECT_CALL(*comMock, send);
  comMock->send(p, 5);

  // test expectations without destroying instance
  ASSERT_TRUE(Mock::VerifyAndClear(solenoidsMock));
  ASSERT_TRUE(Mock::VerifyAndClear(comMock));
}

TEST_F(KnitterTest, test_isr) {
  expected_isr(1);

  // test expectations without destroying instance
  ASSERT_TRUE(Mock::VerifyAndClear(solenoidsMock));
  ASSERT_TRUE(Mock::VerifyAndClear(encodersMock));
}

TEST_F(KnitterTest, test_startKnitting_NoMachine) {
  uint8_t pattern[] = {1};
  Machine_t m = knitter->getMachineType();
  ASSERT_EQ(m, Machine_t::NoMachine);
  ASSERT_TRUE(knitter->initMachine(m) != ErrorCode::success);
  ASSERT_TRUE(
      knitter->startKnitting(0, NUM_NEEDLES[static_cast<uint8_t>(m)] - 1, pattern, false) != ErrorCode::success);

  // test expectations without destroying instance
  ASSERT_TRUE(Mock::VerifyAndClear(solenoidsMock));
}

TEST_F(KnitterTest, test_startKnitting_invalidMachine) {
  uint8_t pattern[] = {1};
  ASSERT_TRUE(knitter->initMachine(Machine_t::NoMachine) != ErrorCode::success);
  ASSERT_TRUE(knitter->startKnitting(0, 1, pattern, false) != ErrorCode::success);

  // test expectations without destroying instance
  ASSERT_TRUE(Mock::VerifyAndClear(solenoidsMock));
}

TEST_F(KnitterTest, test_startKnitting_notReady) {
  uint8_t pattern[] = {1};
  ASSERT_TRUE(knitter->startKnitting(0, NUM_NEEDLES[static_cast<uint8_t>(Machine_t::Kh910)] - 1, pattern,
                                     false) != ErrorCode::success);

  // test expectations without destroying instance
  ASSERT_TRUE(Mock::VerifyAndClear(solenoidsMock));
}

TEST_F(KnitterTest, test_startKnitting_Kh910) {
  get_to_knit(Machine_t::Kh910);

  // test expectations without destroying instance
  ASSERT_TRUE(Mock::VerifyAndClear(solenoidsMock));
  ASSERT_TRUE(Mock::VerifyAndClear(encodersMock));
  ASSERT_TRUE(Mock::VerifyAndClear(beeperMock));
  ASSERT_TRUE(Mock::VerifyAndClear(comMock));
}

TEST_F(KnitterTest, test_startKnitting_Kh270) {
  get_to_knit(Machine_t::Kh270);

  // test expectations without destroying instance
  ASSERT_TRUE(Mock::VerifyAndClear(solenoidsMock));
  ASSERT_TRUE(Mock::VerifyAndClear(encodersMock));
  ASSERT_TRUE(Mock::VerifyAndClear(beeperMock));
  ASSERT_TRUE(Mock::VerifyAndClear(comMock));
}

TEST_F(KnitterTest, test_startKnitting_failures) {
  uint8_t pattern[] = {1};
  get_to_ready(Machine_t::Kh910);

  // `m_stopNeedle` lower than `m_startNeedle`
  ASSERT_TRUE(knitter->startKnitting(1, 0, pattern, false) != ErrorCode::success);

  // `m_stopNeedle` out of range
  ASSERT_TRUE(knitter->startKnitting(0, NUM_NEEDLES[static_cast<uint8_t>(Machine_t::Kh910)], pattern,
                                     false) != ErrorCode::success);

  // null pattern
  ASSERT_TRUE(knitter->startKnitting(0, NUM_NEEDLES[static_cast<uint8_t>(Machine_t::Kh910)] - 1, nullptr,
                                     false) != ErrorCode::success);

  // test expectations without destroying instance
  ASSERT_TRUE(Mock::VerifyAndClear(solenoidsMock));
  ASSERT_TRUE(Mock::VerifyAndClear(encodersMock));
  ASSERT_TRUE(Mock::VerifyAndClear(comMock));
}

TEST_F(KnitterTest, test_setNextLine) {
  // set `m_lineRequested`
  ASSERT_EQ(knitter->setNextLine(1), false);

  expected_dispatch_knit(true);

  // outside of the active needles
  expected_isr(NUM_NEEDLES[static_cast<uint8_t>(Machine_t::Kh910)] + END_OF_LINE_OFFSET_R[static_cast<uint8_t>(Machine_t::Kh910)] + 1 + knitter->getStartOffset(Direction_t::Left));
  EXPECT_CALL(*solenoidsMock, setSolenoid).Times(1);
  expected_dispatch_knit(false);

  // wrong line number
  EXPECT_CALL(*beeperMock, finishedLine).Times(0);
  expect_reqLine();
  ASSERT_EQ(knitter->setNextLine(1), false);

  // correct line number
  EXPECT_CALL(*beeperMock, finishedLine).Times(1);
  ASSERT_EQ(knitter->setNextLine(0), true);

  // `m_lineRequested` has been set to `false`
  ASSERT_EQ(knitter->setNextLine(0), false);

  // test expectations without destroying instance
  ASSERT_TRUE(Mock::VerifyAndClear(solenoidsMock));
  ASSERT_TRUE(Mock::VerifyAndClear(encodersMock));
  ASSERT_TRUE(Mock::VerifyAndClear(beeperMock));
  ASSERT_TRUE(Mock::VerifyAndClear(comMock));
}

TEST_F(KnitterTest, test_knit_Kh910) {
  get_to_ready(Machine_t::Kh910);

  // knit
  uint8_t pattern[] = {1};

  // `m_startNeedle` is greater than `m_pixelToSet`
  EXPECT_CALL(*beeperMock, ready);
  const uint8_t START_NEEDLE = NUM_NEEDLES[static_cast<uint8_t>(Machine_t::Kh910)] - 2;
  const uint8_t STOP_NEEDLE = NUM_NEEDLES[static_cast<uint8_t>(Machine_t::Kh910)] - 1;
  knitter->startKnitting(START_NEEDLE, STOP_NEEDLE, pattern, true);
  EXPECT_CALL(*arduinoMock, digitalWrite(LED_PIN_A, LOW)); // green LED off
  expected_dispatch();

  // first knit
  expect_first_knit();
  expect_indState();
  expected_dispatch_knit(false);

  // no useful position calculated by `calculatePixelAndSolenoid()`
  expected_isr(0, Direction_t::Right, Direction_t::Right, BeltShift::Shifted, Carriage_t::Knit);
  EXPECT_CALL(*solenoidsMock, setSolenoid).Times(0);
  expect_indState();
  expected_dispatch_knit(false);

  // don't set `m_workedonline` to `true`
  const uint8_t OFFSET = END_OF_LINE_OFFSET_R[static_cast<uint8_t>(Machine_t::Kh910)];
  expected_isr(8 + STOP_NEEDLE + OFFSET);
  EXPECT_CALL(*solenoidsMock, setSolenoid);
  expect_indState();
  expected_dispatch_knit(false);

  expected_isr(START_NEEDLE);
  EXPECT_CALL(*solenoidsMock, setSolenoid);
  expect_indState();
  expected_dispatch_knit(false);

  // test expectations without destroying instance
  ASSERT_TRUE(Mock::VerifyAndClear(solenoidsMock));
  ASSERT_TRUE(Mock::VerifyAndClear(encodersMock));
  ASSERT_TRUE(Mock::VerifyAndClear(beeperMock));
  ASSERT_TRUE(Mock::VerifyAndClear(comMock));
}

TEST_F(KnitterTest, test_knit_should_not_request_new_line_if_carriage_goes_back_one_needle_inside_working_area) {
  get_to_ready(Machine_t::Kh910);

  // not interested in beeps for this test
  EXPECT_CALL(*beeperMock, finishedLine).Times(AnyNumber());

  // 0-only pattern for the first row
  uint8_t pattern[25] = { };

  // all solenoids should be set to 0 only for the first row
  EXPECT_CALL(*solenoidsMock, setSolenoid(_, 0)).Times(AnyNumber());

  const uint8_t startNeedle = 0;
  const uint8_t stopNeedle = 100;
  const Carriage carriageType = Carriage::Garter;
  knitter->startKnitting(startNeedle, stopNeedle, pattern, /* continuous_reporting */ false);

  // first knit: should request first line
  EXPECT_CALL(*comMock, send_reqLine);
  knitter->knit();

  // confirm reception of first line
  knitter->setNextLine(0);

  // offset between working needle and m_position, for K carriage moving rightwards
  const int startOffset =
      START_OFFSET[(int)MachineType::Kh910]
                  // yes, Direction::Left is for a carriage moving rightwards
                  [(int)Direction::Left][(int)carriageType];

  int currentPosition;

  // working on a needle near the end of the work area
  expected_isr(currentPosition = 90 + startOffset, Direction::Right, Direction::NoDirection, BeltShift::Regular, carriageType);
  knitter->knit();

  // going back left one needle
  expected_isr(--currentPosition,
    Direction::Left,
    Direction::NoDirection, BeltShift::Regular, carriageType);
  // should not request a new line!
  EXPECT_CALL(*comMock, send_reqLine).Times(0);
  knitter->knit();

  // going rightwards to the last needle in work (stop_needle)
  expected_isr(currentPosition = stopNeedle + startOffset, Direction::Right, Direction::NoDirection, BeltShift::Regular, carriageType);
  // should not request a new line yet
  EXPECT_CALL(*comMock, send_reqLine).Times(0);
  knitter->knit();

  // going rightwards to just outside the work area
  expected_isr(++currentPosition, Direction::Right, Direction::NoDirection, BeltShift::Regular, carriageType);
  // should not request a new line (because of END_OF_LINE_OFFSET_R)
  EXPECT_CALL(*comMock, send_reqLine).Times(0);
  knitter->knit();

  // move rightwards until the line is requested
  bool lineWasRequested = false;
  EXPECT_CALL(*comMock, send_reqLine(1, _)).Times(AtMost(1)).WillOnce(Assign(&lineWasRequested, true));
  while (!lineWasRequested) {
    // safety valve if the line is never requested
    ASSERT_LT(currentPosition, 200);
    // going rightwards one needle at a time
    expected_isr(++currentPosition, Direction::Right, Direction::NoDirection, BeltShift::Regular, carriageType);
    // at some point it should request a new line
    knitter->knit();
  }

  // the line should have been requested at a point where the
  // leftwards needle checker is outside of the work area
  const int leftwardsOffset =
      START_OFFSET[(int)MachineType::Kh910]
                  // yes, Direction::Right is for a carriage moving leftwards
                  [(int)Direction::Right][(int)carriageType];
  ASSERT_GT(currentPosition - leftwardsOffset, stopNeedle);

  // test expectations without destroying instance
  ASSERT_TRUE(Mock::VerifyAndClear(solenoidsMock));
  ASSERT_TRUE(Mock::VerifyAndClear(encodersMock));
  ASSERT_TRUE(Mock::VerifyAndClear(beeperMock));
  ASSERT_TRUE(Mock::VerifyAndClear(comMock));
}

TEST_F(KnitterTest, test_knit_Kh270) {
  get_to_ready(Machine_t::Kh270);

  // knit
  uint8_t pattern[] = {1};

  // `m_startNeedle` is greater than `m_pixelToSet`
  EXPECT_CALL(*beeperMock, ready);
  const uint8_t START_NEEDLE = NUM_NEEDLES[static_cast<uint8_t>(Machine_t::Kh270)] - 2;
  const uint8_t STOP_NEEDLE = NUM_NEEDLES[static_cast<uint8_t>(Machine_t::Kh270)] - 1;
  knitter->startKnitting(START_NEEDLE, STOP_NEEDLE, pattern, true);
  EXPECT_CALL(*arduinoMock, digitalWrite(LED_PIN_A, LOW));
  expected_dispatch();

  // first knit
  expect_first_knit();
  expect_indState();
  expected_dispatch_knit(false);

  // second knit
  expected_isr(START_NEEDLE);
  expect_indState();
  EXPECT_CALL(*solenoidsMock, setSolenoid);
  expected_dispatch_knit(false);

  // no useful position calculated by `calculatePixelAndSolenoid()`
  expected_isr(0, Direction_t::Right, Direction_t::Right, BeltShift::Shifted, Carriage_t::Knit);
  EXPECT_CALL(*solenoidsMock, setSolenoid).Times(0);
  expect_indState();
  expected_dispatch_knit(false);

  // don't set `m_workedonline` to `true`
  const uint8_t OFFSET = END_OF_LINE_OFFSET_R[static_cast<uint8_t>(Machine_t::Kh270)];
  expected_isr(8 + STOP_NEEDLE + OFFSET, Direction_t::Right, Direction_t::Left, BeltShift::Regular, Carriage_t::Knit);
  EXPECT_CALL(*solenoidsMock, setSolenoid);
  expect_indState();
  expected_dispatch_knit(false);

  expected_isr(START_NEEDLE, Direction_t::Right, Direction_t::Left, BeltShift::Regular, Carriage_t::Knit);
  EXPECT_CALL(*solenoidsMock, setSolenoid);
  expect_indState();
  expected_dispatch_knit(false);

  // test expectations without destroying instance
  ASSERT_TRUE(Mock::VerifyAndClear(solenoidsMock));
  ASSERT_TRUE(Mock::VerifyAndClear(encodersMock));
  ASSERT_TRUE(Mock::VerifyAndClear(beeperMock));
  ASSERT_TRUE(Mock::VerifyAndClear(comMock));
}

TEST_F(KnitterTest, test_knit_line_request) {
  // `m_workedOnLine` is set to `true`
  expected_dispatch_knit(true);

  // Position has changed since last call to operate function
  // `m_pixelToSet` is set above `m_stopNeedle` + END_OF_LINE_OFFSET_R
  expected_isr(NUM_NEEDLES[static_cast<uint8_t>(Machine_t::Kh910)] + 8 + END_OF_LINE_OFFSET_R[static_cast<uint8_t>(Machine_t::Kh910)] + 1);

  EXPECT_CALL(*solenoidsMock, setSolenoid);
  expected_dispatch_knit(false);

  // no change in position, no action.
  EXPECT_CALL(*solenoidsMock, setSolenoid).Times(0);
  expected_dispatch_knit(false);

  // test expectations without destroying instance
  ASSERT_TRUE(Mock::VerifyAndClear(solenoidsMock));
  ASSERT_TRUE(Mock::VerifyAndClear(encodersMock));
  ASSERT_TRUE(Mock::VerifyAndClear(beeperMock));
  ASSERT_TRUE(Mock::VerifyAndClear(comMock));
}

TEST_F(KnitterTest, test_knit_lastLine) {  
  expected_dispatch_knit(true);

  // Run one knit inside the working needles.
  EXPECT_CALL(*solenoidsMock, setSolenoid);
  expected_isr(knitter->getStartOffset(Direction_t::Left) + 20,
               Direction::Right, Direction::Left, BeltShift::Regular,
               Carriage::Knit);
  // `m_workedOnLine` is set to true
  expected_dispatch_knit(false);

  // Position has changed since last call to operate function
  // `m_pixelToSet` is above `m_stopNeedle` + END_OF_LINE_OFFSET_R
  expected_isr(
      NUM_NEEDLES[static_cast<uint8_t>(Machine_t::Kh910)] +
          END_OF_LINE_OFFSET_R[static_cast<uint8_t>(Machine_t::Kh910)] + 1 +
          knitter->getStartOffset(Direction_t::Left),
      Direction::Right, Direction::Left, BeltShift::Regular, Carriage::Knit);

  // `m_lastLineFlag` is `true`
  knitter->setLastLine();

  EXPECT_CALL(*solenoidsMock, setSolenoid);
  EXPECT_CALL(*beeperMock, endWork);
  EXPECT_CALL(*solenoidsMock, setSolenoids(SOLENOIDS_BITMASK));
  EXPECT_CALL(*beeperMock, finishedLine);
  expected_dispatch_knit(false);

  // test expectations without destroying instance
  ASSERT_TRUE(Mock::VerifyAndClear(solenoidsMock));
  ASSERT_TRUE(Mock::VerifyAndClear(encodersMock));
  ASSERT_TRUE(Mock::VerifyAndClear(beeperMock));
  ASSERT_TRUE(Mock::VerifyAndClear(comMock));
}

TEST_F(KnitterTest, test_knit_lastLine_and_no_req) {
  get_to_knit(Machine_t::Kh910);

  // Note: probing private data and methods to get full branch coverage.
  knitter->m_stopNeedle = 100;
  uint8_t wanted_pixel =
      knitter->m_stopNeedle + END_OF_LINE_OFFSET_R[static_cast<uint8_t>(Machine_t::Kh910)] + 1;
  knitter->m_firstRun = false;
  knitter->m_currentLineDirection = Direction_t::Right;
  knitter->m_position = wanted_pixel + knitter->getStartOffset(Direction_t::Left);
  knitter->m_lineRequested = false;
  knitter->m_lastLineFlag = true;

  // EXPECT_CALL(*arduinoMock, digitalWrite(LED_PIN_A, 1));
  EXPECT_CALL(*solenoidsMock, setSolenoid);
  EXPECT_CALL(*beeperMock, endWork);
  EXPECT_CALL(*solenoidsMock, setSolenoids(SOLENOIDS_BITMASK));
  EXPECT_CALL(*beeperMock, finishedLine);
  knitter->knit();

  ASSERT_EQ(knitter->getStartOffset(Direction_t::NoDirection), 0);
  knitter->m_carriage = Carriage_t::NoCarriage;
  ASSERT_EQ(knitter->getStartOffset(Direction_t::Right), 0);

  // test expectations without destroying instance
  ASSERT_TRUE(Mock::VerifyAndClear(solenoidsMock));
  ASSERT_TRUE(Mock::VerifyAndClear(encodersMock));
  ASSERT_TRUE(Mock::VerifyAndClear(beeperMock));
  ASSERT_TRUE(Mock::VerifyAndClear(comMock));
}

TEST_F(KnitterTest, test_knit_same_position) {
  expected_dispatch_knit(true);

  // no call to `setSolenoid()` since position was the same
  EXPECT_CALL(*solenoidsMock, setSolenoid).Times(0);
  expected_dispatch_knit(false);

  // test expectations without destroying instance
  ASSERT_TRUE(Mock::VerifyAndClear(solenoidsMock));
  ASSERT_TRUE(Mock::VerifyAndClear(encodersMock));
  ASSERT_TRUE(Mock::VerifyAndClear(beeperMock));
  ASSERT_TRUE(Mock::VerifyAndClear(comMock));
}

TEST_F(KnitterTest, test_knit_new_line) {
  // _workedOnLine is set to true
  expected_dispatch_knit(true);

  // Run one knit inside the working needles.
  EXPECT_CALL(*solenoidsMock, setSolenoid);
  expected_isr(knitter->getStartOffset(Direction_t::Left) + 20,
               Direction::Right, Direction::Left, BeltShift::Regular,
               Carriage::Knit);
  // `m_workedOnLine` is set to true
  expected_dispatch_knit(false);

  // Position has changed since last call to operate function
  // `m_pixelToSet` is above `m_stopNeedle` + END_OF_LINE_OFFSET_R
  expected_isr(
      NUM_NEEDLES[static_cast<uint8_t>(Machine_t::Kh910)] +
          END_OF_LINE_OFFSET_R[static_cast<uint8_t>(Machine_t::Kh910)] + 1 +
          knitter->getStartOffset(Direction_t::Left),
      Direction::Right, Direction::Left, BeltShift::Regular, Carriage::Knit);

  // set `m_lineRequested` to `false`
  EXPECT_CALL(*beeperMock, finishedLine);
  knitter->setNextLine(0);

  EXPECT_CALL(*solenoidsMock, setSolenoid);

  // `reqLine()` is called which calls `send_reqLine()`
  expect_reqLine();
  expected_dispatch_knit(false);

  // test expectations without destroying instance
  ASSERT_TRUE(Mock::VerifyAndClear(solenoidsMock));
  ASSERT_TRUE(Mock::VerifyAndClear(encodersMock));
  ASSERT_TRUE(Mock::VerifyAndClear(beeperMock));
  ASSERT_TRUE(Mock::VerifyAndClear(comMock));
}

TEST_F(KnitterTest, test_calculatePixelAndSolenoid) {
  // initialize
  expected_init_machine(Machine_t::Kh910);
  fsm->setState(OpState::test);
  expected_dispatch_init();

  // new position, different beltShift and active hall
  expected_isr(100, Direction_t::Right, Direction_t::Right, BeltShift::Shifted, Carriage_t::Lace);
  expected_dispatch_test();

  // no direction, need to change position to enter test
  expected_isr(101, Direction_t::NoDirection, Direction_t::Right, BeltShift::Shifted, Carriage_t::Lace);
  expected_dispatch_test();

  // no belt, need to change position to enter test
  expected_isr(100, Direction_t::Right, Direction_t::Right, BeltShift::Unknown, Carriage_t::Lace);
  expected_dispatch_test();

  // no belt on left side, need to change position to enter test
  expected_isr(101, Direction_t::Left, Direction_t::Right, BeltShift::Unknown, Carriage_t::Garter);
  expected_dispatch_test();

  // left Lace carriage
  expected_isr(100, Direction_t::Left, Direction_t::Right, BeltShift::Unknown, Carriage_t::Lace);
  expected_dispatch_test();

  // regular belt on left, need to change position to enter test
  expected_isr(101, Direction_t::Left, Direction_t::Right, BeltShift::Regular, Carriage_t::Garter);
  expected_dispatch_test();

  // shifted belt on left, need to change position to enter test
  expected_isr(100, Direction_t::Left, Direction_t::Right, BeltShift::Shifted, Carriage_t::Garter);
  expected_dispatch_test();

  // off of right end, position is changed
  expected_isr(END_RIGHT[static_cast<uint8_t>(Machine_t::Kh910)], Direction_t::Left, Direction_t::Right, BeltShift::Unknown, Carriage_t::Lace);
  expected_dispatch_test();

  // direction right, have not reached offset
  expected_isr(39, Direction_t::Right, Direction_t::Left, BeltShift::Unknown, Carriage_t::Lace);
  expected_dispatch_test();

  // KH270
  knitter->setMachineType(Machine_t::Kh270);

  // K carriage direction left
  expected_isr(0, Direction_t::Left, Direction_t::Right, BeltShift::Regular, Carriage_t::Knit);
  expected_dispatch_test();

  // K carriage direction right
  expected_isr(END_RIGHT[static_cast<uint8_t>(Machine_t::Kh270)], Direction_t::Right, Direction_t::Left, BeltShift::Regular, Carriage_t::Knit);
  expected_dispatch_test();

  // test expectations without destroying instance
  ASSERT_TRUE(Mock::VerifyAndClear(solenoidsMock));
  ASSERT_TRUE(Mock::VerifyAndClear(encodersMock));
  ASSERT_TRUE(Mock::VerifyAndClear(testerMock));
  ASSERT_TRUE(Mock::VerifyAndClear(comMock));
}

TEST_F(KnitterTest, test_getStartOffset) {
  // out of range values
  knitter->m_carriage = Carriage_t::Knit;
  ASSERT_EQ(knitter->getStartOffset(Direction_t::NoDirection), 0);

  knitter->m_carriage = Carriage_t::NoCarriage;
  ASSERT_EQ(knitter->getStartOffset(Direction_t::Left), 0);
  ASSERT_EQ(knitter->getStartOffset(Direction_t::Right), 0);

  knitter->m_carriage = Carriage_t::Lace;
  knitter->m_machineType = Machine_t::NoMachine;
  ASSERT_EQ(knitter->getStartOffset(Direction_t::Left), 0);
  ASSERT_EQ(knitter->getStartOffset(Direction_t::Right), 0);

  // test expectations without destroying instance
  ASSERT_TRUE(Mock::VerifyAndClear(solenoidsMock));
}

TEST_F(KnitterTest, test_fsm_init_LL) {
  expected_init_machine(Machine_t::Kh910);

  // not ready
  expected_isr(get_position_past_right(), Direction_t::Left, Direction_t::Left);
  expected_dispatch_init();
  ASSERT_EQ(fsm->getState(), OpState::init);

  // test expectations without destroying instance
  ASSERT_TRUE(Mock::VerifyAndClear(solenoidsMock));
  ASSERT_TRUE(Mock::VerifyAndClear(comMock));
  ASSERT_TRUE(Mock::VerifyAndClear(encodersMock));
}

TEST_F(KnitterTest, test_fsm_init_RR) {
  expected_init_machine(Machine_t::Kh910);

  // still not ready
  expected_isr(get_position_past_left(), Direction_t::Right, Direction_t::Right);
  expected_dispatch_init();
  ASSERT_EQ(fsm->getState(), OpState::init);

  // test expectations without destroying instance
  ASSERT_TRUE(Mock::VerifyAndClear(solenoidsMock));
  ASSERT_TRUE(Mock::VerifyAndClear(comMock));
  ASSERT_TRUE(Mock::VerifyAndClear(encodersMock));
}

TEST_F(KnitterTest, test_fsm_init_RL) {
  expected_init_machine(Machine_t::Kh910);

  // Machine is initialized when Left hall sensor
  // is passed in Right direction inside active needles.
  expected_isr(get_position_past_left(), Direction_t::Right, Direction_t::Left);
  expected_get_ready();

  // test expectations without destroying instance
  ASSERT_TRUE(Mock::VerifyAndClear(solenoidsMock));
  ASSERT_TRUE(Mock::VerifyAndClear(comMock));
  ASSERT_TRUE(Mock::VerifyAndClear(encodersMock));
}

TEST_F(KnitterTest, test_fsm_init_LR) {
  expected_init_machine(Machine_t::Kh910);

  // New feature (August 2020): the machine is also initialized
  // when the right Hall sensor is passed in the Left direction.
  expected_isr(get_position_past_right(), Direction_t::Left, Direction_t::Right);
  expected_get_ready();
  ASSERT_EQ(fsm->getState(), OpState::ready);

  // test expectations without destroying instance
  ASSERT_TRUE(Mock::VerifyAndClear(solenoidsMock));
  ASSERT_TRUE(Mock::VerifyAndClear(comMock));
  ASSERT_TRUE(Mock::VerifyAndClear(encodersMock));
}
