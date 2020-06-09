/*!
 * \file knitter.cpp
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
 *    Original Work Copyright 2013-2015 Christian Obersteiner, Andreas Müller
 *    Modified Work Copyright 2020 Sturla Lange
 *    http://ayab-knitting.com
 */

#include <Arduino.h>

#include "board.h"
#include "knitter.h"

#ifdef CLANG_TIDY
// clang-tidy doesn't find these macros for some reason,
// no problem when building or testing though.
constexpr uint8_t UINT8_MAX = 0xFFU;
constexpr uint16_t UINT16_MAX = 0xFFFFU;
#endif

#ifndef AYAB_TESTS
/*!
 * \brief Wrapper for knitter's isr.
 *
 * This is needed since a non-static method cannot be
 * passed to _attachInterrupt_.
 */
void isr_wrapper() {
  extern Knitter *knitter;
  knitter->isr();
}
#endif

/*!
 * \brief Knitter constructor.
 *
 * Initializes the solenoids as well as pins and interrupts.
 */
Knitter::Knitter() : m_beeper(), m_serial_encoding() {

  pinMode(ENC_PIN_A, INPUT);
#ifndef AYAB_TESTS
  // Attaching ENC_PIN_A(=2), Interrupt No. 0
  attachInterrupt(0, isr_wrapper, CHANGE);
#endif

  pinMode(ENC_PIN_B, INPUT);
  pinMode(ENC_PIN_C, INPUT);

  pinMode(LED_PIN_A, OUTPUT);
  pinMode(LED_PIN_B, OUTPUT);
  digitalWrite(LED_PIN_A, 1);
  digitalWrite(LED_PIN_B, 1);

#if DBG_NOMACHINE
  pinMode(DBG_BTN_PIN, INPUT);
#endif

  m_solenoids.init();
}

auto Knitter::getState() -> OpState_t {
  return m_opState;
}

void Knitter::send(uint8_t *payload, size_t length) {
  m_serial_encoding.send(payload, length);
}

void Knitter::isr() {
  // Update machine state data
  m_encoders.encA_interrupt();
  m_position = m_encoders.getPosition();
  m_direction = m_encoders.getDirection();
  m_hallActive = m_encoders.getHallActive();
  m_beltshift = m_encoders.getBeltshift();
  m_carriage = m_encoders.getCarriage();
}

void Knitter::fsm() {
  switch (m_opState) {
  case s_init:
    state_init();
    break;

  case s_ready:
    state_ready();
    break;

  case s_operate:
    state_operate();
    break;

  case s_test:
    state_test();
    break;

  default:
    break;
  }
  m_serial_encoding.update();
}

/*!
 * \brief Enter operate state.
 *
 * \todo sl: Check that functionality is correct after removing always true
 * comparison.
 */
auto Knitter::startOperation(uint8_t startNeedle, uint8_t stopNeedle,
                             bool continuousReportingEnabled, uint8_t *line)
    -> bool {
  bool success = false;
  if (line == nullptr) {
    return success;
  }

  // TODO(sl): Check ok after removed always true comparison.
  if (stopNeedle < NUM_NEEDLES && startNeedle < stopNeedle) {
    if (s_ready == m_opState) {
      // Proceed to next state
      m_opState = s_operate;
      // Assign image width
      m_startNeedle = startNeedle;
      m_stopNeedle = stopNeedle;
      // Continuous Reporting enabled?
      m_continuousReportingEnabled = continuousReportingEnabled;
      // Set pixel data source
      m_lineBuffer = line;

      // Reset variables to start conditions
      m_currentLineNumber = UINT8_MAX; // because counter will
                                       // be increased before request
      m_lineRequested = false;
      m_lastLineFlag = false;
      // TODO(sl): Not used? Can be removed?
      m_lastLinesCountdown = 2;

      Beeper::ready();

      success = true;
    }
  }
  return success;
}

auto Knitter::startTest() -> bool {
  bool success = false;
  if (s_init == m_opState || s_ready == m_opState) {
    m_opState = s_test;
    success = true;
  }
  return success;
}

auto Knitter::setNextLine(uint8_t lineNumber) -> bool {
  bool success = false;
  if (m_lineRequested) {
    // Is there even a need for a new line?
    if (lineNumber == m_currentLineNumber) {
      m_lineRequested = false;
      Beeper::finishedLine();
      success = true;
    } else {
      //  line numbers didnt match -> request again
      reqLine(m_currentLineNumber);
    }
  }
  return success;
}

void Knitter::setLastLine() {
  // lastLineFlag is evaluated in s_operate
  m_lastLineFlag = true;
}

/* Private Methods */

void Knitter::state_init() {
#ifdef DBG_NOMACHINE
  bool state = digitalRead(DBG_BTN_PIN);

  // TODO(Who?): Check if debounce is needed
  if (m_prevState && !state) {
#else
  // Machine is initialized when left hall sensor is passed in Right direction
  if (Right == m_direction && Left == m_hallActive) {
#endif // DBG_NOMACHINE
    m_opState = s_ready;
    m_solenoids.setSolenoids(UINT16_MAX);
    indState(true);
  }

#ifdef DBG_NOMACHINE
  m_prevState = state;
#endif
}

void Knitter::state_ready() {
  digitalWrite(LED_PIN_A, 0);
  // This state is left when the startOperation() method
  // is called successfully by main()
}

void Knitter::state_operate() {
  digitalWrite(LED_PIN_A, 1);

  if (m_firstRun) {
    m_firstRun = false;
    // TODO(Who?): Optimize Delay for various Arduino Models
    delay(START_OPERATION_DELAY);
    Beeper::finishedLine();
    reqLine(++m_currentLineNumber);
  }

#ifdef DBG_NOMACHINE
  bool state = digitalRead(DBG_BTN_PIN);

  // TODO(Who?): Check if debounce is needed
  if (m_prevState && !state) {
    if (!m_lineRequested) {
      reqLine(++m_currentLineNumber);
    }
  }
  m_prevState = state;
#else
  if (m_sOldPosition != m_position) {
    // Only act if there is an actual change of position
    // Store current Encoder position for next call of this function
    m_sOldPosition = m_position;

    if (m_continuousReportingEnabled) {
      // Send current position to GUI
      indState(true);
    }

    if (!calculatePixelAndSolenoid()) {
      // No valid/useful position calculated
      return;
    }

    if ((m_pixelToSet >= m_startNeedle - END_OF_LINE_OFFSET_L) &&
        (m_pixelToSet <= m_stopNeedle + END_OF_LINE_OFFSET_R)) {

      if ((m_pixelToSet >= m_startNeedle) && (m_pixelToSet <= m_stopNeedle)) {
        m_workedOnLine = true;
      }

      // Find the right byte from the currentLine array,
      // then read the appropriate Pixel(/Bit) for the current needle to set
      uint8_t currentByte = m_pixelToSet / 8U;
      bool pixelValue =
          bitRead(m_lineBuffer[currentByte], m_pixelToSet - (8U * currentByte));
      // Write Pixel state to the appropriate needle
      m_solenoids.setSolenoid(m_solenoidToSet, pixelValue);
    } else { // Outside of the active needles
      //  digitalWrite(LED_PIN_B, 0);

      // Reset Solenoids when out of range
      m_solenoids.setSolenoid(m_solenoidToSet, true);

      if (m_workedOnLine) {
        // already worked on the current line -> finished the line
        m_workedOnLine = false;

        if (!m_lineRequested && !m_lastLineFlag) {
          // request new Line from Host
          reqLine(++m_currentLineNumber);
        } else if (m_lastLineFlag) {
          Beeper::endWork();
          m_opState = s_ready;
          m_solenoids.setSolenoids(UINT16_MAX);
          Beeper::finishedLine();
        }
      }
    }
  }
#endif // DBG_NOMACHINE
}

void Knitter::state_test() {
  if (m_sOldPosition != m_position) {
    // Only act if there is an actual change of position
    // Store current Encoder position for next call of this function
    m_sOldPosition = m_position;

    calculatePixelAndSolenoid();
    indState();
  }
}

auto Knitter::calculatePixelAndSolenoid() -> bool {
  uint8_t startOffset = 0;
  bool success = true;

  switch (m_direction) {
  // Calculate the solenoid and pixel to be set
  // Implemented according to machine manual
  // Magic numbers result from machine manual
  // TODO(sl): 16 is number of solenoids? 8 is half? Replace with named
  // constant.
  case Right:
    startOffset = getStartOffset(Left);
    if (m_position >= startOffset) {
      m_pixelToSet = m_position - startOffset;

      if (Regular == m_beltshift) {
        m_solenoidToSet = m_position % 16;
      } else if (Shifted == m_beltshift) {
        m_solenoidToSet = (m_position - 8) % 16;
      }

      if (L == m_carriage) {
        m_pixelToSet = m_pixelToSet + 8;
      }
    } else {
      success = false;
    }
    break;

  case Left:
    startOffset = getStartOffset(Right);
    if (m_position <= (END_RIGHT - startOffset)) {
      m_pixelToSet = m_position - startOffset;

      if (Regular == m_beltshift) {
        m_solenoidToSet = (m_position + 8) % 16;
      } else if (Shifted == m_beltshift) {
        m_solenoidToSet = m_position % 16;
      }

      if (L == m_carriage) {
        m_pixelToSet = m_pixelToSet - 16;
      }
    } else {
      success = false;
    }
    break;

  default:
    success = false;
    break;
  }
  return success;
}

auto Knitter::getStartOffset(const Direction_t direction) -> uint8_t {
  if (direction >= NUM_DIRECTIONS || m_carriage >= NUM_CARRIAGES) {
    return 0U;
  }

  return startOffsetLUT[direction][m_carriage];
}

void Knitter::reqLine(const uint8_t lineNumber) {
  constexpr uint8_t REQLINE_LEN = 2U;
  uint8_t payload[REQLINE_LEN] = {
      reqLine_msgid,
      lineNumber,
  };
  send(static_cast<uint8_t *>(payload), REQLINE_LEN);

  m_lineRequested = true;
}

void Knitter::indState(const bool initState) {
  constexpr uint8_t INDSTATE_LEN = 9U;
  uint16_t leftHallValue = Encoders::getHallValue(Left);
  uint16_t rightHallValue = Encoders::getHallValue(Right);
  uint8_t payload[INDSTATE_LEN] = {
      indState_msgid,
      static_cast<uint8_t>(initState),
      highByte(leftHallValue),
      lowByte(leftHallValue),
      highByte(rightHallValue),
      lowByte(rightHallValue),
      static_cast<uint8_t>(m_carriage),
      static_cast<uint8_t>(m_position),
      static_cast<uint8_t>(m_encoders.getDirection()),
  };
  send(static_cast<uint8_t *>(payload), INDSTATE_LEN);
}

void Knitter::onPacketReceived(const uint8_t *buffer, size_t size) {
  m_serial_encoding.onPacketReceived(buffer, size);
}
