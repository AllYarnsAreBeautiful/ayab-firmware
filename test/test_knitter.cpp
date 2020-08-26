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

    // start in state `s_init`
    expected_isr(NoDirection, NoDirection);
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
    expect_isr(1, dir, hall, Regular, Knit);
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
    // starts in state `s_init`
    ASSERT_TRUE(fsm->getState() == s_init);

    EXPECT_CALL(*solenoidsMock, setSolenoids(SOLENOIDS_BITMASK));
    expect_indState();
    expected_dispatch_init();

    // ends in state `s_ready`
    ASSERT_EQ(fsm->getState(), s_ready);
  }

  void get_to_ready() {
    // Machine is initialized when Left hall sensor
    // is passed in Right direction inside active needles.
    Machine_t m = knitter->getMachineType();
    expected_isr(40 + END_OF_LINE_OFFSET_L[m] + 1, Right, Left, Regular, Knit);
    expected_get_ready();
  }

  void get_to_knit(Machine_t m) {
    get_to_ready();
    uint8_t pattern[] = {1};
    EXPECT_CALL(*beeperMock, ready);
    EXPECT_CALL(*encodersMock, init);
    knitter->startKnitting(m, 0, NUM_NEEDLES[m] - 1, pattern, false);
    expected_dispatch_ready();

    // ends in state `s_knit`
    ASSERT_TRUE(fsm->getState() == s_knit);
  }

  void expected_dispatch_knit(bool first) {
    if (first) {
      get_to_knit(Kh910);
      expect_first_knit();
      EXPECT_CALL(*arduinoMock, digitalWrite(LED_PIN_A, HIGH)); // green LED on
      expected_dispatch();
      return;
    }
    ASSERT_TRUE(fsm->getState() == s_knit);
    EXPECT_CALL(*arduinoMock, digitalWrite(LED_PIN_A, HIGH)); // green LED on
    expected_dispatch();
  }

  void expected_dispatch_init() {
    // starts in state `s_init`
    ASSERT_TRUE(fsm->getState() == s_init);

    EXPECT_CALL(*arduinoMock, digitalWrite(LED_PIN_A, LOW));
    expected_dispatch();
  }

  void expected_dispatch_ready() {
    // starts in state `s_ready`
    ASSERT_TRUE(fsm->getState() == s_ready);

    EXPECT_CALL(*arduinoMock, digitalWrite(LED_PIN_A, LOW));
    expected_dispatch();
  }

  void expected_dispatch_test() {
    // starts in state `s_test`
    ASSERT_TRUE(fsm->getState() == s_test);

    expect_indState();
    EXPECT_CALL(*testerMock, loop);
    expected_dispatch();
  }

  void expect_first_knit() {
    EXPECT_CALL(*arduinoMock, delay(2000));
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
  ASSERT_EQ(m, NoMachine);
  get_to_ready();
  ASSERT_TRUE(
      knitter->startKnitting(m, 0, NUM_NEEDLES[m] - 1, pattern, false) != 0);

  // test expectations without destroying instance
  ASSERT_TRUE(Mock::VerifyAndClear(solenoidsMock));
}

TEST_F(KnitterTest, test_startKnitting_invalidMachine) {
  uint8_t pattern[] = {1};
  get_to_ready();
  ASSERT_TRUE(knitter->startKnitting(NUM_MACHINES, 0, 1, pattern, false) != 0);

  // test expectations without destroying instance
  ASSERT_TRUE(Mock::VerifyAndClear(solenoidsMock));
}

TEST_F(KnitterTest, test_startKnitting_notReady) {
  uint8_t pattern[] = {1};
  ASSERT_TRUE(knitter->startKnitting(Kh910, 0, NUM_NEEDLES[Kh910] - 1, pattern,
                                     false) != 0);

  // test expectations without destroying instance
  ASSERT_TRUE(Mock::VerifyAndClear(solenoidsMock));
}

TEST_F(KnitterTest, test_startKnitting_Kh910) {
  get_to_knit(Kh910);

  // test expectations without destroying instance
  ASSERT_TRUE(Mock::VerifyAndClear(solenoidsMock));
  ASSERT_TRUE(Mock::VerifyAndClear(encodersMock));
  ASSERT_TRUE(Mock::VerifyAndClear(beeperMock));
  ASSERT_TRUE(Mock::VerifyAndClear(comMock));
}

TEST_F(KnitterTest, test_startKnitting_Kh270) {
  get_to_knit(Kh270);

  // test expectations without destroying instance
  ASSERT_TRUE(Mock::VerifyAndClear(solenoidsMock));
  ASSERT_TRUE(Mock::VerifyAndClear(encodersMock));
  ASSERT_TRUE(Mock::VerifyAndClear(beeperMock));
  ASSERT_TRUE(Mock::VerifyAndClear(comMock));
}

TEST_F(KnitterTest, test_startKnitting_failures) {
  uint8_t pattern[] = {1};
  get_to_ready();

  // `m_stopNeedle` lower than `m_startNeedle`
  ASSERT_TRUE(knitter->startKnitting(Kh910, 1, 0, pattern, false) != 0);

  // `m_stopNeedle` out of range
  ASSERT_TRUE(knitter->startKnitting(Kh910, 0, NUM_NEEDLES[Kh910], pattern,
                                     false) != 0);

  // null pattern
  ASSERT_TRUE(knitter->startKnitting(Kh910, 0, NUM_NEEDLES[Kh910] - 1, nullptr,
                                     false) != 0);

  // test expectations without destroying instance
  ASSERT_TRUE(Mock::VerifyAndClear(solenoidsMock));
  ASSERT_TRUE(Mock::VerifyAndClear(encodersMock));
  ASSERT_TRUE(Mock::VerifyAndClear(comMock));
}

TEST_F(KnitterTest, test_setNextLine) {
  ASSERT_EQ(knitter->setNextLine(1), false);

  // set `m_lineRequested`
  EXPECT_CALL(*solenoidsMock, setSolenoid).Times(1);
  expected_dispatch_knit(true);

  // outside of the active needles
  expected_isr(40 + NUM_NEEDLES[Kh910] - 1 + END_OF_LINE_OFFSET_R[Kh910] + 1);
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
  // `m_pixelToSet` gets set to 0
  expected_isr(8);

  // initialize
  expected_get_ready();

  // knit
  uint8_t pattern[] = {1};

  // `m_startNeedle` is greater than `m_pixelToSet`
  EXPECT_CALL(*beeperMock, ready);
  EXPECT_CALL(*encodersMock, init);
  const uint8_t START_NEEDLE = NUM_NEEDLES[Kh910] - 2;
  const uint8_t STOP_NEEDLE = NUM_NEEDLES[Kh910] - 1;
  knitter->startKnitting(Kh910, START_NEEDLE, STOP_NEEDLE, pattern, true);
  EXPECT_CALL(*arduinoMock, digitalWrite(LED_PIN_A, LOW)); // green LED off
  expected_dispatch();

  // first knit
  expect_first_knit();
  expect_indState();
  EXPECT_CALL(*solenoidsMock, setSolenoid);
  expected_dispatch_knit(false);

  // no useful position calculated by `calculatePixelAndSolenoid()`
  expected_isr(100, NoDirection, Right, Shifted, Knit);
  EXPECT_CALL(*solenoidsMock, setSolenoid).Times(0);
  expect_indState();
  expected_dispatch_knit(false);

  // don't set `m_workedonline` to `true`
  const uint8_t OFFSET = END_OF_LINE_OFFSET_R[Kh910];
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

TEST_F(KnitterTest, test_knit_Kh270) {
  // `m_pixelToSet` gets set to 0
  expected_isr(8);

  // initialize
  expected_get_ready();

  // knit
  uint8_t pattern[] = {1};

  // `m_startNeedle` is greater than `m_pixelToSet`
  EXPECT_CALL(*beeperMock, ready);
  EXPECT_CALL(*encodersMock, init);
  const uint8_t START_NEEDLE = NUM_NEEDLES[Kh270] - 2;
  const uint8_t STOP_NEEDLE = NUM_NEEDLES[Kh270] - 1;
  knitter->startKnitting(Kh270, START_NEEDLE, STOP_NEEDLE, pattern, true);
  EXPECT_CALL(*arduinoMock, digitalWrite(LED_PIN_A, LOW));
  expected_dispatch();

  // first knit
  EXPECT_CALL(*arduinoMock, digitalWrite(LED_PIN_B, 0))
      .Times(AtLeast(0)); // yellow LED off
  EXPECT_CALL(*arduinoMock, digitalWrite(LED_PIN_B, 1))
      .Times(AtLeast(1)); // yellow LED on
  EXPECT_CALL(*arduinoMock, digitalWrite(LED_PIN_B, 0))
      .Times(AtLeast(0)); // yellow LED off
  expect_first_knit();
  expect_indState();
  EXPECT_CALL(*solenoidsMock, setSolenoid);
  expected_dispatch_knit(false);

  // no useful position calculated by `calculatePixelAndSolenoid()`
  expected_isr(60, NoDirection, Right, Shifted, Knit);
  EXPECT_CALL(*solenoidsMock, setSolenoid).Times(0);
  expect_indState();
  expected_dispatch_knit(false);

  // don't set `m_workedonline` to `true`
  const uint8_t OFFSET = END_OF_LINE_OFFSET_R[Kh270];
  expected_isr(8 + STOP_NEEDLE + OFFSET, Right, Left, Regular, Knit);
  EXPECT_CALL(*solenoidsMock, setSolenoid);
  expect_indState();
  expected_dispatch_knit(false);

  expected_isr(START_NEEDLE, Right, Left, Regular, Knit);
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
  EXPECT_CALL(*solenoidsMock, setSolenoid);

  // `m_workedOnLine` is set to `true`
  expected_dispatch_knit(true);

  // Position has changed since last call to operate function
  // `m_pixelToSet` is set above `m_stopNeedle` + END_OF_LINE_OFFSET_R
  expected_isr(NUM_NEEDLES[Kh910] + 8 + END_OF_LINE_OFFSET_R[Kh910] + 1);

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
  EXPECT_CALL(*solenoidsMock, setSolenoid);

  // `m_workedOnLine` is set to true
  expected_dispatch_knit(true);

  // Position has changed since last call to operate function
  // `m_pixelToSet` is above `m_stopNeedle` + END_OF_LINE_OFFSET_R
  expected_isr(NUM_NEEDLES[Kh910] + 8 + END_OF_LINE_OFFSET_R[Kh910] + 1);

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
  get_to_knit(Kh910);

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

  // EXPECT_CALL(*arduinoMock, digitalWrite(LED_PIN_A, 1));
  EXPECT_CALL(*solenoidsMock, setSolenoid);
  EXPECT_CALL(*beeperMock, endWork);
  EXPECT_CALL(*solenoidsMock, setSolenoids(SOLENOIDS_BITMASK));
  EXPECT_CALL(*beeperMock, finishedLine);
  knitter->knit();

  ASSERT_EQ(knitter->getStartOffset(NUM_DIRECTIONS), 0);
  knitter->m_carriage = NUM_CARRIAGES;
  ASSERT_EQ(knitter->getStartOffset(Right), 0);

  // test expectations without destroying instance
  ASSERT_TRUE(Mock::VerifyAndClear(solenoidsMock));
  ASSERT_TRUE(Mock::VerifyAndClear(encodersMock));
  ASSERT_TRUE(Mock::VerifyAndClear(beeperMock));
  ASSERT_TRUE(Mock::VerifyAndClear(comMock));
}

TEST_F(KnitterTest, test_knit_same_position) {
  EXPECT_CALL(*solenoidsMock, setSolenoid);
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
  EXPECT_CALL(*solenoidsMock, setSolenoid);

  // _workedOnLine is set to true
  expected_dispatch_knit(true);

  // Position has changed since last call to operate function
  // `m_pixelToSet` is above `m_stopNeedle` + END_OF_LINE_OFFSET_R
  expected_isr(NUM_NEEDLES[Kh910] + 8 + END_OF_LINE_OFFSET_R[Kh910] + 1);

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
  knitter->setMachineType(Kh910);
  fsm->setState(s_test);
  expected_dispatch_init();

  // new position, different beltShift and active hall
  expected_isr(100, Right, Right, Shifted, Lace);
  expected_dispatch_test();

  // no direction, need to change position to enter test
  expected_isr(101, NoDirection, Right, Shifted, Lace);
  expected_dispatch_test();

  // no belt, need to change position to enter test
  expected_isr(100, Right, Right, Unknown, Lace);
  expected_dispatch_test();

  // no belt on left side, need to change position to enter test
  expected_isr(101, Left, Right, Unknown, Garter);
  expected_dispatch_test();

  // left Lace carriage
  expected_isr(100, Left, Right, Unknown, Lace);
  expected_dispatch_test();

  // regular belt on left, need to change position to enter test
  expected_isr(101, Left, Right, Regular, Garter);
  expected_dispatch_test();

  // shifted belt on left, need to change position to enter test
  expected_isr(100, Left, Right, Shifted, Garter);
  expected_dispatch_test();

  // off of right end, position is changed
  expected_isr(END_RIGHT[Kh910], Left, Right, Unknown, Lace);
  expected_dispatch_test();

  // direction right, have not reached offset
  expected_isr(39, Right, Left, Unknown, Lace);
  expected_dispatch_test();

  // KH270
  knitter->setMachineType(Kh270);

  // K carriage direction left
  expected_isr(0, Left, Right, Regular, Knit);
  expected_dispatch_test();

  // K carriage direction right
  expected_isr(END_RIGHT[Kh270], Right, Left, Regular, Knit);
  expected_dispatch_test();

  // test expectations without destroying instance
  ASSERT_TRUE(Mock::VerifyAndClear(solenoidsMock));
  ASSERT_TRUE(Mock::VerifyAndClear(encodersMock));
  ASSERT_TRUE(Mock::VerifyAndClear(testerMock));
  ASSERT_TRUE(Mock::VerifyAndClear(comMock));
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

  // test expectations without destroying instance
  ASSERT_TRUE(Mock::VerifyAndClear(solenoidsMock));
}

TEST_F(KnitterTest, test_fsm_init_LL) {
  // not ready
  expected_isr(Left, Left);
  expected_dispatch_init();
  ASSERT_EQ(fsm->getState(), s_init);

  // test expectations without destroying instance
  ASSERT_TRUE(Mock::VerifyAndClear(solenoidsMock));
  ASSERT_TRUE(Mock::VerifyAndClear(comMock));
  ASSERT_TRUE(Mock::VerifyAndClear(encodersMock));
}

TEST_F(KnitterTest, test_fsm_init_RR) {
  // still not ready
  expected_isr(Right, Right);
  expected_dispatch_init();
  ASSERT_EQ(fsm->getState(), s_init);

  // test expectations without destroying instance
  ASSERT_TRUE(Mock::VerifyAndClear(solenoidsMock));
  ASSERT_TRUE(Mock::VerifyAndClear(comMock));
  ASSERT_TRUE(Mock::VerifyAndClear(encodersMock));
}

TEST_F(KnitterTest, test_fsm_init_RL) {
  // Machine is initialized when Left hall sensor
  // is passed in Right direction inside active needles.
  expected_isr(Right, Left);
  expected_get_ready();

  // test expectations without destroying instance
  ASSERT_TRUE(Mock::VerifyAndClear(solenoidsMock));
  ASSERT_TRUE(Mock::VerifyAndClear(comMock));
  ASSERT_TRUE(Mock::VerifyAndClear(encodersMock));
}

TEST_F(KnitterTest, test_fsm_init_LR) {
  // New feature (August 2020): the machine is also initialized
  // when the right Hall sensor is passed in the Left direction.
  expected_isr(Left, Right);
  expected_get_ready();
  ASSERT_EQ(fsm->getState(), s_ready);

  // test expectations without destroying instance
  ASSERT_TRUE(Mock::VerifyAndClear(solenoidsMock));
  ASSERT_TRUE(Mock::VerifyAndClear(comMock));
  ASSERT_TRUE(Mock::VerifyAndClear(encodersMock));
}
