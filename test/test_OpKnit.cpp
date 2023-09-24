/*!`
 * \file test_opKnit.cpp
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
 *    Modified Work Copyright 2020-3 Sturla Lange, Tom Price
 *    http://ayab-knitting.com
 */

#include <gtest/gtest.h>

#include <board.h>
#include <opKnit.h>

#include <beeper_mock.h>
#include <com_mock.h>
#include <encoders_mock.h>
#include <fsm_mock.h>
#include <solenoids_mock.h>

#include <opIdle_mock.h>
#include <opInit_mock.h>
#include <opReady_mock.h>
#include <opTest_mock.h>

using ::testing::_;
using ::testing::AtLeast;
using ::testing::Mock;
using ::testing::Return;
using ::testing::TypedEq;

extern OpKnit *opKnit;
extern Fsm *fsm;

extern BeeperMock *beeper;
extern ComMock *com;
extern EncodersMock *encoders;
extern SolenoidsMock *solenoids;

extern OpIdleMock *opIdle;
extern OpInitMock *opInit;
extern OpTestMock *opTest;
extern OpReadyMock *opReady;

class OpKnitTest : public ::testing::Test {
protected:
  void SetUp() override {
    arduinoMock = arduinoMockInstance();

    // pointers to global instances
    beeperMock = beeper;
    comMock = com;
    encodersMock = encoders;
    solenoidsMock = solenoids;

    opIdleMock = opIdle;
    opInitMock = opInit;
    opReadyMock = opReady;
    opTestMock = opTest;

    // The global instances do not get destroyed at the end of each test.
    // Ordinarily the mock instances would be local and such behaviour would
    // cause a memory leak. We must notify the test that this is not the case.
    Mock::AllowLeak(beeperMock);
    Mock::AllowLeak(comMock);
    Mock::AllowLeak(encodersMock);
    Mock::AllowLeak(solenoidsMock);

    Mock::AllowLeak(opIdleMock);
    Mock::AllowLeak(opInitMock);
    Mock::AllowLeak(opReadyMock);
    Mock::AllowLeak(opTestMock);

    // start in state `OpIdle`
    fsm->init();
    opIdle->init();
    opInit->init();
    expect_opKnit_init();
    opKnit->init();
    expected_cacheISR(Direction_t::NoDirection, Direction_t::NoDirection);
  }

  void TearDown() override {
    releaseArduinoMock();
  }

  ArduinoMock *arduinoMock;
  BeeperMock *beeperMock;
  ComMock *comMock;
  EncodersMock *encodersMock;
  SolenoidsMock *solenoidsMock;

  OpIdleMock *opIdleMock;
  OpInitMock *opInitMock;
  OpReadyMock *opReadyMock;
  OpTestMock *opTestMock;

  uint8_t get_position_past_left() {
    return (END_LEFT_PLUS_OFFSET[static_cast<uint8_t>(encoders->getMachineType())] + GARTER_SLOP) + 1;
  }

  uint8_t get_position_past_right() {
    return (END_RIGHT_MINUS_OFFSET[static_cast<uint8_t>(encoders->getMachineType())] - GARTER_SLOP) - 1;
  }

  void expect_opKnit_init() {
    EXPECT_CALL(*arduinoMock, pinMode(ENC_PIN_A, INPUT));
    EXPECT_CALL(*arduinoMock, pinMode(ENC_PIN_B, INPUT));
    EXPECT_CALL(*arduinoMock, pinMode(ENC_PIN_C, INPUT));

    EXPECT_CALL(*arduinoMock, pinMode(LED_PIN_A, OUTPUT));
    EXPECT_CALL(*arduinoMock, pinMode(LED_PIN_B, OUTPUT));

    EXPECT_CALL(*arduinoMock, digitalWrite(LED_PIN_A, HIGH)); // green LED on
    EXPECT_CALL(*arduinoMock, digitalWrite(LED_PIN_B, HIGH)); // yellow LED on

    EXPECT_CALL(*solenoidsMock, init);
  }

  void expect_cacheISR(uint16_t pos, Direction_t dir, Direction_t hall,
                  BeltShift_t belt, Carriage_t carriage) {
    EXPECT_CALL(*encodersMock, getPosition).WillRepeatedly(Return(pos));
    EXPECT_CALL(*encodersMock, getDirection).WillRepeatedly(Return(dir));
    EXPECT_CALL(*encodersMock, getHallActive).WillRepeatedly(Return(hall));
    EXPECT_CALL(*encodersMock, getBeltShift).WillRepeatedly(Return(belt));
    EXPECT_CALL(*encodersMock, getCarriage).WillRepeatedly(Return(carriage));
  }

  void expected_cacheISR(uint16_t pos, Direction_t dir, Direction_t hall,
                    BeltShift_t belt, Carriage_t carriage) {
    expect_cacheISR(pos, dir, hall, belt, carriage);
    fsm->cacheEncoders();
  }

  void expect_cacheISR(Direction_t dir, Direction_t hall) {
    expect_cacheISR(1, dir, hall, BeltShift::Regular, Carriage_t::Knit);
  }

  void expected_cacheISR(uint8_t pos, Direction_t dir, Direction_t hall) {
    expect_cacheISR(pos, dir, hall, BeltShift::Regular, Carriage_t::Knit);
    fsm->cacheEncoders();
  }

  void expected_cacheISR(Direction_t dir, Direction_t hall) {
    expect_cacheISR(dir, hall);
    fsm->cacheEncoders();
  }

  void expect_cacheISR(uint16_t pos) {
    expect_cacheISR(pos, Direction_t::Right, Direction_t::Left, BeltShift::Regular, Carriage_t::Garter);
  }

  void expected_cacheISR(uint16_t pos) {
    expect_cacheISR(pos);
    fsm->cacheEncoders();
  }

  void expected_cacheISR() {
    expected_cacheISR(1);
  }

  void expect_reqLine() {
    EXPECT_CALL(*comMock, send_reqLine);
  }

  void expect_indState() {
    EXPECT_CALL(*comMock, send_indState);
  }

  void expected_dispatch() {
    fsm->update();
  }

  void expected_get_ready() {
    // start in state `OpInit`
    ASSERT_EQ(fsm->getState(), opInit);

    EXPECT_CALL(*solenoidsMock, setSolenoids(SOLENOIDS_BITMASK));
    expect_indState();
    ASSERT_EQ(opKnit->isReady(), true);
    fsm->setState(opReady);

    // transition to state `OpReady`
    expected_dispatch_init();
    ASSERT_EQ(fsm->getState(), opReady);
  }

  void expected_init_machine(Machine_t m) {
    // starts in state `OpIdle`
    fsm->setMachineType(m);
    fsm->setState(opInitMock);
    EXPECT_CALL(*opIdleMock, end);
    EXPECT_CALL(*opInitMock, begin);
    expected_dispatch_idle();

    ASSERT_EQ(fsm->getState(), opInit);
  }

  void get_to_ready(Machine_t m) {
    expected_init_machine(m);
    // Machine is initialized when Left hall sensor
    // is passed in Right direction inside active needles.
    uint8_t position = get_position_past_left();
    expected_cacheISR(position, Direction_t::Right, Direction_t::Left);
    expected_get_ready();
  }

  void get_to_knit(Machine_t m) {
    EXPECT_CALL(*encodersMock, init);
    get_to_ready(m);
    uint8_t pattern[] = {1};
    EXPECT_CALL(*beeperMock, ready);
    ASSERT_EQ(opKnit->startKnitting(0, NUM_NEEDLES[static_cast<uint8_t>(m)] - 1, pattern, false), Err_t::Success);
    expected_dispatch_ready();

    // ends in state `OpKnit`
    ASSERT_TRUE(fsm->getState() == opKnit);
  }

  void expected_dispatch_knit(bool first) {
    if (first) {
      get_to_knit(Machine_t::Kh910);
      expect_first_knit();
      //EXPECT_CALL(*arduinoMock, digitalWrite(LED_PIN_A, HIGH)); // green LED on
      expected_dispatch();
      return;
    }
    ASSERT_TRUE(fsm->getState() == opKnit);
    //EXPECT_CALL(*arduinoMock, digitalWrite(LED_PIN_A, HIGH)); // green LED on
    expected_dispatch();
  }

  void expected_dispatch_idle() {
    // starts in state `OpIdle`
    ASSERT_EQ(fsm->getState(), opIdle);

    //EXPECT_CALL(*arduinoMock, digitalWrite(LED_PIN_A, LOW));
    expected_dispatch();
  }

  void expected_dispatch_init() {
    // starts in state `OpInit`
    ASSERT_EQ(fsm->getState(), opInit);

    //EXPECT_CALL(*arduinoMock, digitalWrite(LED_PIN_A, LOW));
    expected_dispatch();
  }

  void expected_dispatch_ready() {
    // starts in state `OpReady`
    ASSERT_TRUE(fsm->getState() == opReady);

    //EXPECT_CALL(*arduinoMock, digitalWrite(LED_PIN_A, LOW));
    expected_dispatch();
  }

  void expected_dispatch_test() {
    // starts in state `OpTest`
    ASSERT_EQ(fsm->getState(), opTest);

    //expect_indState();
    EXPECT_CALL(*opTestMock, update);
    expected_dispatch();
  }

  void expect_first_knit() {
    EXPECT_CALL(*arduinoMock, delay(START_KNITTING_DELAY));
    EXPECT_CALL(*beeperMock, finishedLine);
    expect_reqLine();
  }
};

TEST_F(OpKnitTest, test_send) {
  uint8_t p[] = {1, 2, 3, 4, 5};
  EXPECT_CALL(*comMock, send);
  comMock->send(p, 5);

  // test expectations without destroying instance
  ASSERT_TRUE(Mock::VerifyAndClear(comMock));
}

TEST_F(OpKnitTest, test_com) {
  const uint8_t cnf[] = {static_cast<uint8_t>(API_t::cnfLine)};
  EXPECT_CALL(*comMock, h_cnfLine);
  opKnit->com(cnf, 1);

  const uint8_t unrec[] = {0xFF};
  EXPECT_CALL(*comMock, h_unrecognized);
  opKnit->com(unrec, 1);

  // test expectations without destroying instance
  ASSERT_TRUE(Mock::VerifyAndClear(comMock));
}

TEST_F(OpKnitTest, test_encodePosition) {
  opKnit->m_sOldPosition = fsm->getPosition();
  EXPECT_CALL(*comMock, send_indState).Times(0);
  opKnit->encodePosition();

  opKnit->m_sOldPosition += 1;
  EXPECT_CALL(*comMock, send_indState).Times(1);
  opKnit->encodePosition();

  // test expectations without destroying instance
  ASSERT_TRUE(Mock::VerifyAndClear(comMock));
}

TEST_F(OpKnitTest, test_cacheISR) {
  expected_cacheISR();

  // test expectations without destroying instance
  ASSERT_TRUE(Mock::VerifyAndClear(encodersMock));
}

TEST_F(OpKnitTest, test_init_machine) {
  expected_init_machine(Machine_t::Kh910);

  // test expectations without destroying instance
  ASSERT_TRUE(Mock::VerifyAndClear(opIdleMock));
  ASSERT_TRUE(Mock::VerifyAndClear(opInitMock));
}

TEST_F(OpKnitTest, test_startKnitting_NoMachine) {
  uint8_t pattern[] = {1};
  Machine_t m = fsm->getMachineType();
  ASSERT_EQ(m, Machine_t::NoMachine);

  opKnit->begin();
  ASSERT_TRUE(
      opKnit->startKnitting(0, NUM_NEEDLES[static_cast<uint8_t>(m)] - 1, pattern, false) != Err_t::Success);
}

TEST_F(OpKnitTest, test_startKnitting_Kh910) {
  get_to_knit(Machine_t::Kh910);

  // test expectations without destroying instance
  ASSERT_TRUE(Mock::VerifyAndClear(beeperMock));
  ASSERT_TRUE(Mock::VerifyAndClear(comMock));
  ASSERT_TRUE(Mock::VerifyAndClear(encodersMock));
  ASSERT_TRUE(Mock::VerifyAndClear(solenoidsMock));
  ASSERT_TRUE(Mock::VerifyAndClear(opIdleMock));
  ASSERT_TRUE(Mock::VerifyAndClear(opInitMock));
  ASSERT_TRUE(Mock::VerifyAndClear(opReadyMock));
}

TEST_F(OpKnitTest, test_startKnitting_Kh270) {
  get_to_knit(Machine_t::Kh270);

  // test expectations without destroying instance
  ASSERT_TRUE(Mock::VerifyAndClear(beeperMock));
  ASSERT_TRUE(Mock::VerifyAndClear(comMock));
  ASSERT_TRUE(Mock::VerifyAndClear(encodersMock));
  ASSERT_TRUE(Mock::VerifyAndClear(solenoidsMock));
  ASSERT_TRUE(Mock::VerifyAndClear(opInitMock));
}

TEST_F(OpKnitTest, test_startKnitting_failures) {
  uint8_t pattern[] = {1};
  get_to_ready(Machine_t::Kh910);

  // `m_stopNeedle` lower than `m_startNeedle`
  ASSERT_TRUE(opKnit->startKnitting(1, 0, pattern, false) != Err_t::Success);

  // `m_stopNeedle` out of range
  ASSERT_TRUE(opKnit->startKnitting(0, NUM_NEEDLES[static_cast<uint8_t>(Machine_t::Kh910)], pattern,
                                     false) != Err_t::Success);

  // null pattern
  ASSERT_TRUE(opKnit->startKnitting(0, NUM_NEEDLES[static_cast<uint8_t>(Machine_t::Kh910)] - 1, nullptr,
                                     false) != Err_t::Success);

  // test expectations without destroying instance
  ASSERT_TRUE(Mock::VerifyAndClear(solenoidsMock));
  ASSERT_TRUE(Mock::VerifyAndClear(encodersMock));
  ASSERT_TRUE(Mock::VerifyAndClear(comMock));
}

TEST_F(OpKnitTest, test_setNextLine) {
  // set `m_lineRequested`
  ASSERT_EQ(opKnit->setNextLine(1), false);

  expected_dispatch_knit(true);

  // outside of the active needles
  expected_cacheISR(NUM_NEEDLES[static_cast<uint8_t>(Machine_t::Kh910)] + END_OF_LINE_OFFSET_R[static_cast<uint8_t>(Machine_t::Kh910)] + 1 + opKnit->getStartOffset(Direction_t::Left));
  EXPECT_CALL(*solenoidsMock, setSolenoid).Times(1);
  expected_dispatch_knit(false);

  // wrong line number
  EXPECT_CALL(*beeperMock, finishedLine).Times(0);
  expect_reqLine();
  ASSERT_EQ(opKnit->setNextLine(1), false);

  // correct line number
  EXPECT_CALL(*beeperMock, finishedLine).Times(1);
  ASSERT_EQ(opKnit->setNextLine(0), true);

  // `m_lineRequested` has been set to `false`
  ASSERT_EQ(opKnit->setNextLine(0), false);

  // test expectations without destroying instance
  ASSERT_TRUE(Mock::VerifyAndClear(solenoidsMock));
  ASSERT_TRUE(Mock::VerifyAndClear(encodersMock));
  ASSERT_TRUE(Mock::VerifyAndClear(beeperMock));
  ASSERT_TRUE(Mock::VerifyAndClear(comMock));
}

TEST_F(OpKnitTest, test_knit_Kh910) {
  get_to_ready(Machine_t::Kh910);

  // knit
  uint8_t pattern[] = {1};

  // `m_startNeedle` is greater than `m_pixelToSet`
  EXPECT_CALL(*beeperMock, ready);
  const uint8_t START_NEEDLE = NUM_NEEDLES[static_cast<uint8_t>(Machine_t::Kh910)] - 2;
  const uint8_t STOP_NEEDLE = NUM_NEEDLES[static_cast<uint8_t>(Machine_t::Kh910)] - 1;
  opKnit->startKnitting(START_NEEDLE, STOP_NEEDLE, pattern, true);
  //EXPECT_CALL(*arduinoMock, digitalWrite(LED_PIN_A, LOW)); // green LED off
  expected_dispatch();

  // first knit
  expect_first_knit();
  expect_indState();
  expected_dispatch_knit(false);

  // no useful position calculated by `calculatePixelAndSolenoid()`
  expected_cacheISR(100, Direction_t::NoDirection, Direction_t::Right, BeltShift::Shifted, Carriage_t::Knit);
  EXPECT_CALL(*solenoidsMock, setSolenoid).Times(0);
  expect_indState();
  expected_dispatch_knit(false);

  // don't set `m_workedonline` to `true`
  const uint8_t OFFSET = END_OF_LINE_OFFSET_R[static_cast<uint8_t>(Machine_t::Kh910)];
  expected_cacheISR(8 + STOP_NEEDLE + OFFSET);
  EXPECT_CALL(*solenoidsMock, setSolenoid);
  expect_indState();
  expected_dispatch_knit(false);

  expected_cacheISR(START_NEEDLE);
  EXPECT_CALL(*solenoidsMock, setSolenoid);
  expect_indState();
  expected_dispatch_knit(false);

  // test expectations without destroying instance
  ASSERT_TRUE(Mock::VerifyAndClear(solenoidsMock));
  ASSERT_TRUE(Mock::VerifyAndClear(encodersMock));
  ASSERT_TRUE(Mock::VerifyAndClear(beeperMock));
  ASSERT_TRUE(Mock::VerifyAndClear(comMock));
}

TEST_F(OpKnitTest, test_knit_Kh270) {
  get_to_ready(Machine_t::Kh270);

  // knit
  uint8_t pattern[] = {1};

  // `m_startNeedle` is greater than `m_pixelToSet`
  EXPECT_CALL(*beeperMock, ready);
  const uint8_t START_NEEDLE = NUM_NEEDLES[static_cast<uint8_t>(Machine_t::Kh270)] - 2;
  const uint8_t STOP_NEEDLE = NUM_NEEDLES[static_cast<uint8_t>(Machine_t::Kh270)] - 1;
  opKnit->startKnitting(START_NEEDLE, STOP_NEEDLE, pattern, true);
  //EXPECT_CALL(*arduinoMock, digitalWrite(LED_PIN_A, LOW));
  expected_dispatch();

  // first knit
  expect_first_knit();
  expect_indState();
  expected_dispatch_knit(false);

  // second knit
  expected_cacheISR(START_NEEDLE);
  expect_indState();
  EXPECT_CALL(*solenoidsMock, setSolenoid);
  expected_dispatch_knit(false);

  // no useful position calculated by `calculatePixelAndSolenoid()`
  expected_cacheISR(60, Direction_t::NoDirection, Direction_t::Right, BeltShift::Shifted, Carriage_t::Knit);
  EXPECT_CALL(*solenoidsMock, setSolenoid).Times(0);
  expect_indState();
  expected_dispatch_knit(false);

  // don't set `m_workedonline` to `true`
  const uint8_t OFFSET = END_OF_LINE_OFFSET_R[static_cast<uint8_t>(Machine_t::Kh270)];
  expected_cacheISR(8 + STOP_NEEDLE + OFFSET, Direction_t::Right, Direction_t::Left, BeltShift::Regular, Carriage_t::Knit);
  EXPECT_CALL(*solenoidsMock, setSolenoid);
  expect_indState();
  expected_dispatch_knit(false);

  expected_cacheISR(START_NEEDLE, Direction_t::Right, Direction_t::Left, BeltShift::Regular, Carriage_t::Knit);
  EXPECT_CALL(*solenoidsMock, setSolenoid);
  expect_indState();
  expected_dispatch_knit(false);

  // test expectations without destroying instance
  ASSERT_TRUE(Mock::VerifyAndClear(solenoidsMock));
  ASSERT_TRUE(Mock::VerifyAndClear(encodersMock));
  ASSERT_TRUE(Mock::VerifyAndClear(beeperMock));
  ASSERT_TRUE(Mock::VerifyAndClear(comMock));
}

TEST_F(OpKnitTest, test_knit_line_request) {
  // `m_workedOnLine` is set to `true`
  expected_dispatch_knit(true);

  // Position has changed since last call to operate function
  // `m_pixelToSet` is set above `m_stopNeedle` + END_OF_LINE_OFFSET_R
  expected_cacheISR(NUM_NEEDLES[static_cast<uint8_t>(Machine_t::Kh910)] + 8 + END_OF_LINE_OFFSET_R[static_cast<uint8_t>(Machine_t::Kh910)] + 1);

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

TEST_F(OpKnitTest, test_knit_lastLine) {
  expected_dispatch_knit(true);

  // Run one knit inside the working needles.
  EXPECT_CALL(*solenoidsMock, setSolenoid);
  expected_cacheISR(opKnit->getStartOffset(Direction_t::Left) + 20);
  // `m_workedOnLine` is set to true
  expected_dispatch_knit(false);

  // Position has changed since last call to operate function
  // `m_pixelToSet` is above `m_stopNeedle` + END_OF_LINE_OFFSET_R
  expected_cacheISR(NUM_NEEDLES[static_cast<uint8_t>(Machine_t::Kh910)] + END_OF_LINE_OFFSET_R[static_cast<uint8_t>(Machine_t::Kh910)] + 1 + opKnit->getStartOffset(Direction_t::Left));

  // `m_lastLineFlag` is `true`
  opKnit->setLastLine();

  EXPECT_CALL(*solenoidsMock, setSolenoid);
  EXPECT_CALL(*beeperMock, endWork);
  EXPECT_CALL(*solenoidsMock, setSolenoids(SOLENOIDS_BITMASK));
  //EXPECT_CALL(*beeperMock, finishedLine);
  expected_dispatch_knit(false);

  // test expectations without destroying instance
  ASSERT_TRUE(Mock::VerifyAndClear(solenoidsMock));
  ASSERT_TRUE(Mock::VerifyAndClear(encodersMock));
  ASSERT_TRUE(Mock::VerifyAndClear(beeperMock));
  ASSERT_TRUE(Mock::VerifyAndClear(comMock));
}

TEST_F(OpKnitTest, test_knit_lastLine_and_no_req) {
  expected_dispatch_knit(true);

  // Run one knit inside the working needles.
  EXPECT_CALL(*solenoidsMock, setSolenoid);
  expected_cacheISR(opKnit->getStartOffset(Direction_t::Left) + 20);
  // `m_workedOnLine` is set to true
  expected_dispatch_knit(false);

  // Position has changed since last call to operate function
  // `m_pixelToSet` is above `m_stopNeedle` + END_OF_LINE_OFFSET_R
  expected_cacheISR(NUM_NEEDLES[static_cast<uint8_t>(Machine_t::Kh910)] + END_OF_LINE_OFFSET_R[static_cast<uint8_t>(Machine_t::Kh910)] + 1 + opKnit->getStartOffset(Direction_t::Left));

  // `m_lastLineFlag` is `true`
  opKnit->setLastLine();

  // Note: probing private data and methods to get full branch coverage.
  opKnit->m_lineRequested = false;

  // EXPECT_CALL(*arduinoMock, digitalWrite(LED_PIN_A, 1));
  EXPECT_CALL(*solenoidsMock, setSolenoid);
  EXPECT_CALL(*beeperMock, endWork);
  EXPECT_CALL(*solenoidsMock, setSolenoids(SOLENOIDS_BITMASK));
  //EXPECT_CALL(*beeperMock, finishedLine);
  expected_dispatch_knit(false);

  // test expectations without destroying instance
  ASSERT_TRUE(Mock::VerifyAndClear(solenoidsMock));
  ASSERT_TRUE(Mock::VerifyAndClear(encodersMock));
  ASSERT_TRUE(Mock::VerifyAndClear(beeperMock));
  ASSERT_TRUE(Mock::VerifyAndClear(comMock));
}

TEST_F(OpKnitTest, test_knit_same_position) {
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

TEST_F(OpKnitTest, test_knit_new_line) {
  // _workedOnLine is set to true
  expected_dispatch_knit(true);

  // Run one knit inside the working needles.
  EXPECT_CALL(*solenoidsMock, setSolenoid);
  expected_cacheISR(opKnit->getStartOffset(Direction_t::Left) + 20);
  // `m_workedOnLine` is set to true
  expected_dispatch_knit(false);

  // Position has changed since last call to operate function
  // `m_pixelToSet` is above `m_stopNeedle` + END_OF_LINE_OFFSET_R
  expected_cacheISR(NUM_NEEDLES[static_cast<uint8_t>(Machine_t::Kh910)] + END_OF_LINE_OFFSET_R[static_cast<uint8_t>(Machine_t::Kh910)] + 1 + opKnit->getStartOffset(Direction_t::Left));

  // set `m_lineRequested` to `false`
  EXPECT_CALL(*beeperMock, finishedLine);
  opKnit->setNextLine(0);

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

TEST_F(OpKnitTest, test_calculatePixelAndSolenoid) {
  // initialize
  expected_init_machine(Machine_t::Kh910);
  fsm->setState(opTest);
  expected_dispatch_init();

  // new position, different beltShift and active hall
  expected_cacheISR(100, Direction_t::Right, Direction_t::Right, BeltShift::Shifted, Carriage_t::Lace);
  expected_dispatch_test();

  // no direction, need to change position to enter test
  expected_cacheISR(101, Direction_t::NoDirection, Direction_t::Right, BeltShift::Shifted, Carriage_t::Lace);
  expected_dispatch_test();

  // no belt, need to change position to enter test
  expected_cacheISR(100, Direction_t::Right, Direction_t::Right, BeltShift::Unknown, Carriage_t::Lace);
  expected_dispatch_test();

  // no belt on left side, need to change position to enter test
  expected_cacheISR(101, Direction_t::Left, Direction_t::Right, BeltShift::Unknown, Carriage_t::Garter);
  expected_dispatch_test();

  // left Lace carriage
  expected_cacheISR(100, Direction_t::Left, Direction_t::Right, BeltShift::Unknown, Carriage_t::Lace);
  expected_dispatch_test();

  // regular belt on left, need to change position to enter test
  expected_cacheISR(101, Direction_t::Left, Direction_t::Right, BeltShift::Regular, Carriage_t::Garter);
  expected_dispatch_test();

  // shifted belt on left, need to change position to enter test
  expected_cacheISR(100, Direction_t::Left, Direction_t::Right, BeltShift::Shifted, Carriage_t::Garter);
  expected_dispatch_test();

  // off of right end, position is changed
  expected_cacheISR(END_RIGHT[static_cast<uint8_t>(Machine_t::Kh910)], Direction_t::Left, Direction_t::Right, BeltShift::Unknown, Carriage_t::Lace);
  expected_dispatch_test();

  // direction right, have not reached offset
  expected_cacheISR(39, Direction_t::Right, Direction_t::Left, BeltShift::Unknown, Carriage_t::Lace);
  expected_dispatch_test();

  // KH270
  fsm->setMachineType(Machine_t::Kh270);

  // K carriage direction left
  expected_cacheISR(0, Direction_t::Left, Direction_t::Right, BeltShift::Regular, Carriage_t::Knit);
  expected_dispatch_test();

  // K carriage direction right
  expected_cacheISR(END_RIGHT[static_cast<uint8_t>(Machine_t::Kh270)], Direction_t::Right, Direction_t::Left, BeltShift::Regular, Carriage_t::Knit);
  expected_dispatch_test();

  // test expectations without destroying instance
  ASSERT_TRUE(Mock::VerifyAndClear(solenoidsMock));
  ASSERT_TRUE(Mock::VerifyAndClear(encodersMock));
  ASSERT_TRUE(Mock::VerifyAndClear(opTestMock));
  ASSERT_TRUE(Mock::VerifyAndClear(comMock));
}

TEST_F(OpKnitTest, test_getStartOffset) {
  // out of range values
  fsm->m_carriage = Carriage_t::Knit;
  ASSERT_EQ(opKnit->getStartOffset(Direction_t::NoDirection), 0);

  fsm->m_carriage = Carriage_t::NoCarriage;
  ASSERT_EQ(opKnit->getStartOffset(Direction_t::Left), 0);
  ASSERT_EQ(opKnit->getStartOffset(Direction_t::Right), 0);

  fsm->m_carriage = Carriage_t::Lace;
  fsm->m_machineType = Machine_t::NoMachine;
  ASSERT_EQ(opKnit->getStartOffset(Direction_t::Left), 0);
  ASSERT_EQ(opKnit->getStartOffset(Direction_t::Right), 0);
}

TEST_F(OpKnitTest, test_op_init_LL) {
  expected_init_machine(Machine_t::Kh910);

  // not ready
  expected_cacheISR(get_position_past_right(), Direction_t::Left, Direction_t::Left);
  expected_dispatch_init();
  ASSERT_EQ(fsm->getState(), opInit);

  // test expectations without destroying instance
  ASSERT_TRUE(Mock::VerifyAndClear(solenoidsMock));
  ASSERT_TRUE(Mock::VerifyAndClear(comMock));
  ASSERT_TRUE(Mock::VerifyAndClear(encodersMock));
}

TEST_F(OpKnitTest, test_op_init_RR) {
  expected_init_machine(Machine_t::Kh910);

  // still not ready
  expected_cacheISR(get_position_past_left(), Direction_t::Right, Direction_t::Right);
  expected_dispatch_init();
  ASSERT_EQ(fsm->getState(), opInit);

  // test expectations without destroying instance
  ASSERT_TRUE(Mock::VerifyAndClear(solenoidsMock));
  ASSERT_TRUE(Mock::VerifyAndClear(comMock));
  ASSERT_TRUE(Mock::VerifyAndClear(encodersMock));
}

TEST_F(OpKnitTest, test_op_init_RL) {
  expected_init_machine(Machine_t::Kh910);

  // Machine is initialized when Left hall sensor
  // is passed in Right direction inside active needles.
  expected_cacheISR(get_position_past_left(), Direction_t::Right, Direction_t::Left);
  expected_get_ready();

  // test expectations without destroying instance
  ASSERT_TRUE(Mock::VerifyAndClear(solenoidsMock));
  ASSERT_TRUE(Mock::VerifyAndClear(comMock));
  ASSERT_TRUE(Mock::VerifyAndClear(encodersMock));
}

TEST_F(OpKnitTest, test_op_init_LR) {
  expected_init_machine(Machine_t::Kh910);

  // New feature (August 2020): the machine is also initialized
  // when the right Hall sensor is passed in the Left direction.
  expected_cacheISR(get_position_past_right(), Direction_t::Left, Direction_t::Right);
  expected_get_ready();
  ASSERT_EQ(fsm->getState(), opReady);

  // test expectations without destroying instance
  ASSERT_TRUE(Mock::VerifyAndClear(solenoidsMock));
  ASSERT_TRUE(Mock::VerifyAndClear(comMock));
  ASSERT_TRUE(Mock::VerifyAndClear(encodersMock));
}
/*
  void expected_isready(Direction_t dir, Direction_t hall, uint8_t position) {
    fsm->m_direction = dir;
    fsm->m_hallActive = hall;
    fsm->m_position = position;
  }

TEST_F(FsmTest, test_update_init) {
  // Get to state `OpInit`
  fsm->setState(opInitMock);
  EXPECT_CALL(*opInit, begin);
  expected_update_idle();
  ASSERT_EQ(fsm->getState(), opInitMock);

  // no transition to state `OpReady`
  expected_isready(Direction_t::Left, Direction_t::Left, 0);
  expected_update_init();
  ASSERT_TRUE(fsm->getState() == opInitMock);

  // no transition to state `OpReady`
  expected_isready(Direction_t::Right, Direction_t::Right, 0);
  expected_update_init();
  ASSERT_TRUE(fsm->getState() == opInitMock);

  // transition to state `OpReady`
  expected_isready(Direction_t::Left, Direction_t::Right, positionPassedRight);
  expect_get_ready();
  expected_update();
  ASSERT_EQ(fsm->getState(), opReadyMock);

  // get to state `OpInit`
  fsm->setState(opInitMock);
  expected_update_ready();

  // transition to state `OpReady`
  expected_isready(Direction_t::Right, Direction_t::Left, positionPassedLeft);
  expect_get_ready();
  expected_update();
  ASSERT_TRUE(fsm->getState() == opReadyMock);

  // test expectations without destroying instance
  ASSERT_TRUE(Mock::VerifyAndClear(comMock));
  ASSERT_TRUE(Mock::VerifyAndClear(solenoidsMock));
  ASSERT_TRUE(Mock::VerifyAndClear(opIdleMock));
  ASSERT_TRUE(Mock::VerifyAndClear(opInitMock));
}
*/
