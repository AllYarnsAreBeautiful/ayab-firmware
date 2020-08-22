/*!`
 * \file knitter.h
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

#ifndef KNITTER_H_
#define KNITTER_H_

#include "beeper.h"
#include "com.h"
#include "encoders.h"
#include "solenoids.h"
#include "tester.h"

// API constants
constexpr uint8_t INDSTATE_LEN = 9U;
constexpr uint8_t REQLINE_LEN = 2U;

enum OpState { s_init, s_ready, s_knit, s_test };
using OpState_t = enum OpState;

class KnitterInterface {
public:
  virtual ~KnitterInterface(){};

  // any methods that need to be mocked should go here
  virtual void init() = 0;
  virtual void fsm() = 0;
  virtual void setUpInterrupt() = 0;
  virtual void isr() = 0;
  virtual bool startKnitting(Machine_t machineType, uint8_t startNeedle,
                             uint8_t stopNeedle, uint8_t *pattern_start,
                             bool continuousReportingEnabled) = 0;
  virtual bool startTest(Machine_t machineType) = 0;
  virtual uint8_t getStartOffset(const Direction_t direction) = 0;
  virtual Machine_t getMachineType() = 0;
  virtual bool setNextLine(uint8_t lineNumber) = 0;
  virtual void setLastLine() = 0;
  virtual void setMachineType(Machine_t) = 0;
  virtual void setState(OpState_t state) = 0;
};

// Singleton container class for static methods.
// Dependency injection is enabled using a pointer
// to a global instance of either `Knitter` or `KnitterMock`
// both of which classes implement the pure virtual methods
// of the `KnitterInterface` class.

class GlobalKnitter final {
private:
  // singleton class so private constructor is appropriate
  GlobalKnitter() = default;

public:
  // pointer to global instance whose methods are implemented
  static KnitterInterface *m_instance;

  static void init();
  static void fsm();
  static void setUpInterrupt();
#ifndef AYAB_TESTS
  static void isr();
#endif
  static bool startKnitting(Machine_t machineType, uint8_t startNeedle,
                            uint8_t stopNeedle, uint8_t *pattern_start,
                            bool continuousReportingEnabled);
  static bool startTest(Machine_t machineType);
  static uint8_t getStartOffset(const Direction_t direction);
  static Machine_t getMachineType();
  static bool setNextLine(uint8_t lineNumber);
  static void setLastLine();
  static void setMachineType(Machine_t);
  static void setState(OpState_t state);
};

class Knitter : public KnitterInterface {
#if AYAB_TESTS
  FRIEND_TEST(KnitterTest, test_init);
  FRIEND_TEST(KnitterTest, test_getStartOffset);
  FRIEND_TEST(KnitterTest, test_operate_lastline);
  FRIEND_TEST(KnitterTest, test_operate_lastline_and_no_req);
  FRIEND_TEST(KnitterTest, test_fsm_default_case);
  FRIEND_TEST(KnitterTest, test_fsm_init_LL);
  FRIEND_TEST(KnitterTest, test_fsm_init_RR);
  FRIEND_TEST(KnitterTest, test_fsm_init_RL);
  FRIEND_TEST(KnitterTest, test_fsm_ready);
  FRIEND_TEST(KnitterTest, test_fsm_test);
  FRIEND_TEST(KnitterTest, test_fsm_test_quit);
  FRIEND_TEST(KnitterTest, test_startKnitting_NoMachine);
  FRIEND_TEST(KnitterTest, test_startKnitting_notReady);
  FRIEND_TEST(KnitterTest, test_startTest_in_init);
  FRIEND_TEST(KnitterTest, test_startTest_in_ready);
  FRIEND_TEST(KnitterTest, test_startTest_in_knit);
  FRIEND_TEST(KnitterTest, test_setNextLine);
#endif
  friend class Tester;

public:
  void init();
  void fsm();
  void setUpInterrupt();
  void isr();
  bool startKnitting(Machine_t machineType, uint8_t startNeedle,
                     uint8_t stopNeedle, uint8_t *pattern_start,
                     bool continuousReportingEnabled);
  bool startTest(Machine_t machineType);
  uint8_t getStartOffset(const Direction_t direction);
  Machine_t getMachineType();
  bool setNextLine(uint8_t lineNumber);
  void setLastLine();
  void setMachineType(Machine_t);
  void setState(OpState_t state);

private:
  void state_init();
  static void state_ready();
  void state_knit();
  void state_test();

  bool calculatePixelAndSolenoid();
  void reqLine(uint8_t lineNumber);
  void indState(bool initState = false);
  void stopKnitting();
  OpState_t getState() const;

  // machine state
  OpState_t m_opState;

  // job parameters
  Machine_t m_machineType;
  uint8_t m_startNeedle;
  uint8_t m_stopNeedle;
  uint8_t *m_lineBuffer;
  bool m_continuousReportingEnabled;

  // current machine state
  uint8_t m_position;
  Direction_t m_direction;
  Direction_t m_hallActive;
  BeltShift_t m_beltShift;
  Carriage_t m_carriage;

  bool m_lineRequested;
  uint8_t m_currentLineNumber;
  bool m_lastLineFlag;

  uint8_t m_sOldPosition;
  bool m_firstRun;
  bool m_workedOnLine;
#ifdef DBG_NOMACHINE
  bool m_prevState;
#endif

  // resulting needle data
  uint8_t m_solenoidToSet;
  uint8_t m_pixelToSet;
};

#endif // KNITTER_H_
