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
 *    Original Work Copyright 2013 Christian Obersteiner, Andreas Müller
 *    Modified Work Copyright 2020 Sturla Lange
 *    http://ayab-knitting.com
 */

#ifndef KNITTER_H_
#define KNITTER_H_

#include <Arduino.h>

#include <PacketSerial.h>

#include "beeper.h"
#include "encoders.h"
#include "serial_encoding.h"
#include "solenoids.h"

// Machine constants
constexpr uint8_t NUM_NEEDLES = 200U;
constexpr uint8_t END_OF_LINE_OFFSET_L = 12U;
constexpr uint8_t END_OF_LINE_OFFSET_R = 12U;

constexpr uint8_t startOffsetLUT[NUM_DIRECTIONS][NUM_CARRIAGES] = {
    // NC,  K,  L,  G
    {0, 0, 0, 0},    // NoDirection
    {0, 40, 40, 8},  // Left
    {0, 16, 16, 32}, // Right
};

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
  void send(uint8_t *payload, size_t length);
  void onPacketReceived(const uint8_t *buffer, size_t size);

private:
  Solenoids m_solenoids;
  Encoders m_encoders;
  Beeper m_beeper;
  SerialEncoding m_serial_encoding;

  OpState_t m_opState = s_init;

  bool m_lastLineFlag = false;
  // TODO(sl): Not used? Can be removed?
  uint8_t m_lastLinesCountdown = 0U;

  // Job Parameters
  uint8_t m_startNeedle = 0U;
  uint8_t m_stopNeedle = 0U;
  bool m_continuousReportingEnabled = false;
  bool m_lineRequested = false;
  uint8_t m_currentLineNumber = 0U;
  uint8_t *m_lineBuffer = nullptr;

  // current machine state
  uint8_t m_position = 0U;
  Direction_t m_direction = NoDirection;
  Direction_t m_hallActive = NoDirection;
  Beltshift_t m_beltshift = Unknown;
  Carriage_t m_carriage = NoCarriage;

  uint8_t m_sOldPosition = 0U;
  bool m_firstRun = true;
  bool m_workedOnLine = false;
#ifdef DBG_NOMACHINE
  bool m_prevState = false;
#endif

  // Resulting needle data
  uint8_t m_solenoidToSet = 0U;
  uint8_t m_pixelToSet = 0U;

  void state_init();
  static void state_ready();
  void state_operate();
  void state_test();

  auto calculatePixelAndSolenoid() -> bool;
  auto getStartOffset(Direction_t direction) -> uint8_t;

  void reqLine(uint8_t lineNumber);
  void indState(bool initState = false);
};

#endif // KNITTER_H_
