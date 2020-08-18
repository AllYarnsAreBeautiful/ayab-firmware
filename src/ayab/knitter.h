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
#include "encoders.h"
#include "hw_test.h"
#include "serial_encoding.h"
#include "solenoids.h"

// API constants
constexpr uint8_t INDSTATE_LEN = 9U;
constexpr uint8_t REQLINE_LEN = 2U;

enum OpState { s_init, s_ready, s_operate, s_test };
using OpState_t = enum OpState;

/*!
 * \brief The knitting finite state machine.
 *
 * Orchestrates the beeper, hall sensors,
 * encoders, and serial communication.
 */
class Knitter {
#if AYAB_TESTS
  FRIEND_TEST(KnitterTest, test_constructor);
  FRIEND_TEST(KnitterTest, test_fsm_default_case);
  FRIEND_TEST(KnitterTest, test_getStartOffset);
  FRIEND_TEST(KnitterTest, test_operate_lastline);
  FRIEND_TEST(KnitterTest, test_operate_lastline_and_no_req);
  FRIEND_TEST(KnitterTest, test_quit_hw_test);
#endif
  friend class HardwareTest;

public:
  Knitter();

  void isr();
  void fsm();
  void setUpInterrupt();
  bool startOperation(Machine_t machineType, uint8_t startNeedle,
                      uint8_t stopNeedle, uint8_t *pattern_start,
                      bool continuousReportingEnabled);

  void send(uint8_t *payload, size_t length);
  void sendMsg(AYAB_API_t id, const char *msg);
  void sendMsg(AYAB_API_t id, char *msg);
  void onPacketReceived(const uint8_t *buffer, size_t size);

  OpState_t getState() const;
  void state_init();
  static void state_ready();
  void state_operate();
  void state_test();
  void setQuitFlag(bool flag);

  bool startTest(Machine_t machineType);

  uint8_t getStartOffset(const Direction_t direction) const;
  Machine_t getMachineType() const;

  bool setNextLine(uint8_t lineNumber);
  void setLastLine();

  // for testing purposes only
  void setMachineType(Machine_t);
  void setState(OpState_t state);
  void setSolenoids(uint16_t state);
  void setSolenoid(uint8_t solenoid, uint8_t state);

private:
  Solenoids m_solenoids;
  Encoders m_encoders;
  Beeper m_beeper;
  SerialEncoding m_serial_encoding;

  // machine state
  OpState_t m_opState = s_init;
  bool m_quitFlag = false;

  // job parameters
  Machine_t m_machineType = NoMachine;
  uint8_t m_startNeedle = 0U;
  uint8_t m_stopNeedle = 0U;
  uint8_t *m_lineBuffer = nullptr;
  bool m_continuousReportingEnabled = false;

  // current machine state
  uint8_t m_position = 0U;
  Direction_t m_direction = NoDirection;
  Direction_t m_hallActive = NoDirection;
  Beltshift_t m_beltshift = Unknown;
  Carriage_t m_carriage = NoCarriage;

  bool m_lineRequested = false;
  uint8_t m_currentLineNumber = 0U;
  bool m_lastLineFlag = false;

  uint8_t m_sOldPosition = 0U;
  bool m_firstRun = true;
  bool m_workedOnLine = false;
#ifdef DBG_NOMACHINE
  bool m_prevState = false;
#endif

  // resulting needle data
  uint8_t m_solenoidToSet = 0U;
  uint8_t m_pixelToSet = 0U;

  bool calculatePixelAndSolenoid();

  void reqLine(uint8_t lineNumber);
  void indState(bool initState = false);

  void stopOperation();
  /* uint8_t m_lastLinesCountdown = 0U; */
};

extern Knitter *knitter;

#endif // KNITTER_H_
