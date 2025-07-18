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

class KnitterInterface {
public:
  virtual ~KnitterInterface() = default;

  // any methods that need to be mocked should go here
  virtual void init() = 0;
  virtual void setUpInterrupt() = 0;
  virtual void isr() = 0;
  virtual Err_t startKnitting(uint8_t startNeedle,
                              uint8_t stopNeedle, uint8_t *pattern_start,
                              bool continuousReportingEnabled) = 0;
  virtual Err_t initMachine(Machine_t machine) = 0;
  virtual void encodePosition() = 0;
  virtual bool isReady() = 0;
  virtual void knit() = 0;
  virtual void indState(Err_t error = ErrorCode::success) = 0;
  virtual uint8_t getStartOffset(const Direction_t direction) = 0;
  virtual Machine_t getMachineType() = 0;
  virtual bool setNextLine(uint8_t lineNumber) = 0;
  virtual void setLastLine() = 0;
  virtual void setMachineType(Machine_t) = 0;
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
  static void setUpInterrupt();
#ifndef AYAB_TESTS
  static void isr();
#endif
  static Err_t startKnitting(uint8_t startNeedle,
                             uint8_t stopNeedle, uint8_t *pattern_start,
                             bool continuousReportingEnabled);
  static Err_t initMachine(Machine_t machine);
  static void encodePosition();
  static bool isReady();
  static void knit();
  static void indState(Err_t error = ErrorCode::success);
  static uint8_t getStartOffset(const Direction_t direction);
  static Machine_t getMachineType();
  static bool setNextLine(uint8_t lineNumber);
  static void setLastLine();
  static void setMachineType(Machine_t);
};

class Knitter : public KnitterInterface {
public:
  void init() final;
  void setUpInterrupt() final;
  void isr() final;
  Err_t startKnitting(uint8_t startNeedle,
                      uint8_t stopNeedle, uint8_t *pattern_start,
                      bool continuousReportingEnabled) final;
  Err_t initMachine(Machine_t machine) final;
  void encodePosition() final;
  bool isReady() final;
  void knit() final;
  void indState(Err_t error = ErrorCode::success) final;
  uint8_t getStartOffset(const Direction_t direction) final;
  Machine_t getMachineType() final;
  bool setNextLine(uint8_t lineNumber) final;
  void setLastLine() final;
  void setMachineType(Machine_t) final;

private:
  void reqLine(uint8_t lineNumber);
  bool calculatePixelAndSolenoid();
  bool isLineFinished();
  void stopKnitting() const;

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
  Direction m_currentLineDirection;
  bool m_lastLineFlag;

  uint8_t m_sOldPosition;
  bool m_firstRun;
  Direction_t m_lastHall;
#ifdef DBG_NOMACHINE
  bool m_prevState;
#endif

  // resulting needle data
  uint8_t m_solenoidToSet;
  uint8_t m_pixelToSet;

#if AYAB_TESTS
  // Note: ideally tests would only rely on the public interface.
  FRIEND_TEST(KnitterTest, test_getStartOffset);
  FRIEND_TEST(KnitterTest, test_knit_lastLine_and_no_req);
#endif
};

#endif // KNITTER_H_
