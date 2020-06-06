/*!
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
 *    Copyright 2013 Christian Obersteiner, Andreas Müller
 *    http://ayab-knitting.com
 */

#ifndef KNITTER_H_
#define KNITTER_H_

#include <Arduino.h>

#include <PacketSerial.h>

#include "beeper.h"
#include "encoders.h"
#include "solenoids.h"

// Machine constants
constexpr uint8_t NUM_NEEDLES = 200;
constexpr uint8_t END_OF_LINE_OFFSET_L = 12;
constexpr uint8_t END_OF_LINE_OFFSET_R = 12;

enum OpState { s_init = 0, s_ready = 1, s_operate = 2, s_test = 3 };

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
  FRIEND_TEST(KnitterTest, test_operate_lastline_and_no_req);
#endif
public:
  Knitter();

  void isr();
  void fsm();
  auto startOperation(uint8_t startNeedle, uint8_t stopNeedle,
                      bool continuousReportingEnabled, uint8_t *line) -> bool;
  auto startTest() -> bool;
  auto setNextLine(uint8_t lineNumber) -> bool;
  void setLastLine();

  auto getState() -> OpState_t;
  void send(uint8_t payload[], size_t length);

private:
  Solenoids m_solenoids;
  Encoders m_encoders;
  Beeper m_beeper;

  OpState_t m_opState = s_init;
  SLIPPacketSerial m_packetSerial;

  bool m_lastLineFlag;
  uint8_t m_lastLinesCountdown;

  // Job Parameters
  uint8_t m_startNeedle = 0;
  uint8_t m_stopNeedle = 0;
  bool m_continuousReportingEnabled;
  bool m_lineRequested = false;
  uint8_t m_currentLineNumber = 0;
  uint8_t *m_lineBuffer;

  // current machine state
  uint8_t m_position;
  Direction_t m_direction = NoDirection;
  Direction_t m_hallActive;
  Beltshift_t m_beltshift;
  Carriage_t m_carriage;

  uint8_t m_sOldPosition = 0;
  bool m_firstRun = true;
  bool m_workedOnLine = false;
#ifdef DBG_NOMACHINE
  bool m_prevState = false;
#endif

  // Resulting needle data
  uint8_t m_solenoidToSet;
  uint8_t m_pixelToSet;

  void state_init();
  void state_ready();
  void state_operate();
  void state_test();

  auto calculatePixelAndSolenoid() -> bool;
  auto getStartOffset(Direction_t) -> uint8_t;

  void reqLine(uint8_t lineNumber);
  void indState(bool initState = false);
};

#endif // KNITTER_H_
