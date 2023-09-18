/*!`
 * \file opKnit.h
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

#ifndef OP_KNIT_H_
#define OP_KNIT_H_

#include "encoders.h"
#include "fsm.h"
#include "op.h"

class OpKnitInterface : public OpInterface {
public:
  virtual ~OpKnitInterface() = default;

  // any methods that need to be mocked should go here
  virtual Err_t startKnitting(uint8_t startNeedle,
                              uint8_t stopNeedle, uint8_t *pattern_start,
                              bool continuousReportingEnabled) = 0;
  virtual void encodePosition() = 0;
  virtual bool isReady() = 0;
  virtual void doKnitting() = 0;
  virtual uint8_t getStartOffset(const Direction_t direction) = 0;
  virtual bool setNextLine(uint8_t lineNumber) = 0;
  virtual void setLastLine() = 0;
};

// Singleton container class for static methods.
// Dependency injection is enabled using a pointer
// to a global instance of either `OpKnit` or `OpKnitMock`
// both of which classes implement the pure virtual methods
// of the `OpKnitInterface` class.

class GlobalOpKnit final {
private:
  // singleton class so private constructor is appropriate
  GlobalOpKnit() = default;

public:
  // pointer to global instance whose methods are implemented
  static OpKnitInterface *m_instance;

  static void init();
  static Err_t begin();
  static void update();
  static void com(const uint8_t *buffer, size_t size);
  static void end();

  static Err_t startKnitting(uint8_t startNeedle,
                             uint8_t stopNeedle, uint8_t *pattern_start,
                             bool continuousReportingEnabled);
  static void encodePosition();
  static bool isReady();
  static void doKnitting();
  static uint8_t getStartOffset(const Direction_t direction);
  static bool setNextLine(uint8_t lineNumber);
  static void setLastLine();
};

class OpKnit : public OpKnitInterface {
public:
  void init() final;
  Err_t begin() final;
  void update() final;
  void com(const uint8_t *buffer, size_t size) final;
  void end() final;

  Err_t startKnitting(uint8_t startNeedle,
                      uint8_t stopNeedle, uint8_t *pattern_start,
                      bool continuousReportingEnabled) final;
  void encodePosition() final;
  bool isReady() final;
  void doKnitting() final;
  uint8_t getStartOffset(const Direction_t direction) final;
  bool setNextLine(uint8_t lineNumber) final;
  void setLastLine() final;

private:
  void reqLine(uint8_t lineNumber);
  bool calculatePixelAndSolenoid();

  // job parameters
  uint8_t m_startNeedle;
  uint8_t m_stopNeedle;
  uint8_t *m_lineBuffer;
  bool m_continuousReportingEnabled;

  // current machine state
  bool m_lineRequested;
  uint8_t m_currentLineNumber;
  bool m_lastLineFlag;

  uint8_t m_sOldPosition;
  bool m_firstRun;
  bool m_workedOnLine;
  Direction_t m_lastHall;
#ifdef DBG_NOMACHINE
  bool m_prevState;
#endif

  // resulting needle data
  uint8_t m_solenoidToSet;
  uint8_t m_pixelToSet;

#if AYAB_TESTS
  FRIEND_TEST(OpKnitTest, test_getStartOffset);
  FRIEND_TEST(OpKnitTest, test_knit_lastLine_and_no_req);
#endif
};

#endif // OP_KNIT_H_
