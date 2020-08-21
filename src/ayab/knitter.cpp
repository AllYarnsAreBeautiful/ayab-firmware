/*!
 * \file knitter.cpp
 * \brief Singleton class containing methods for the finite state machine
 *    that co-ordinates the AYAB firmware.
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
 *    Modified Work Copyright 2020 Sturla Lange, Tom Price
 *    http://ayab-knitting.com
 */

#include <Arduino.h>

#include "beeper.h"
#include "board.h"
#include "com.h"
#include "knitter.h"
#include "tester.h"

#ifdef CLANG_TIDY
// clang-tidy doesn't find these macros for some reason,
// no problem when building or testing though.
constexpr uint8_t UINT8_MAX = 0xFFU;
constexpr uint16_t UINT16_MAX = 0xFFFFU;
#endif

/*!
 * \brief Knitter constructor.
 *
 * Initializes the solenoids as well as pins and interrupts.
 */
void Knitter::init() {
  pinMode(ENC_PIN_A, INPUT);
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
  setUpInterrupt();

  // explicitly initialize members
  m_opState = s_init;
  m_machineType = NoMachine;
  m_startNeedle = 0U;
  m_stopNeedle = 0U;
  m_lineBuffer = nullptr;
  m_continuousReportingEnabled = false;
  m_position = 0U;
  m_direction = NoDirection;
  m_hallActive = NoDirection;
  m_beltshift = Unknown;
  m_carriage = NoCarriage;
  m_lineRequested = false;
  m_currentLineNumber = 0U;
  m_lastLineFlag = false;
  m_sOldPosition = 0U;
  m_firstRun = true;
  m_workedOnLine = false;
  m_solenoidToSet = 0U;
  m_pixelToSet = 0U;
}

void Knitter::setUpInterrupt() {
  // (re-)attach ENC_PIN_A(=2), interrupt #0
  detachInterrupt(0);
#ifndef AYAB_TESTS
  attachInterrupt(0, GlobalKnitter::isr, CHANGE);
#endif // AYAB_TESTS
}

/*!
 * \brief Interrupt service routine.
 *
 * Update machine state data.
 * Must execute as fast as possible.
 * Machine type assumed valid.
 */
void Knitter::isr() {
  // update machine state data
  m_encoders.encA_interrupt();
  m_position = m_encoders.getPosition();
  m_direction = m_encoders.getDirection();
  m_hallActive = m_encoders.getHallActive();
  m_beltshift = m_encoders.getBeltshift();
  m_carriage = m_encoders.getCarriage();
}

/*!
 * \brief Dispatch on machine state
 *
 * \todo TP: add error state(s)
 */
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
  GlobalCom::update();
}

/*!
 * \brief Enter operate state.
 *
 * \todo sl: check that functionality is correct after removing always true
 * comparison.
 */
bool Knitter::startOperation(Machine_t machineType, uint8_t startNeedle,
                             uint8_t stopNeedle, uint8_t *pattern_start,
                             bool continuousReportingEnabled) {
  if ((m_opState != s_ready) || (machineType == NoMachine) ||
      (machineType >= NUM_MACHINES) || (pattern_start == nullptr) ||
      (startNeedle >= stopNeedle) || (stopNeedle >= NUM_NEEDLES[machineType])) {
    // TODO(TP): error code
    return false;
  }

  // record argument values
  m_machineType = machineType;
  m_startNeedle = startNeedle;
  m_stopNeedle = stopNeedle;
  m_lineBuffer = pattern_start;
  m_continuousReportingEnabled = continuousReportingEnabled;

  // reset variables to start conditions
  m_currentLineNumber = UINT8_MAX; // because counter will
                                   // be incremented before request
  m_lineRequested = false;
  m_lastLineFlag = false;

  // initialize encoders
  m_encoders.init(machineType);

  // proceed to next state
  m_opState = s_operate;
  GlobalBeeper::ready();

  // Attaching ENC_PIN_A, Interrupt #0
  // This interrupt cannot be enabled until
  // the machine type has been validated.
  /*
  // `digitalPinToInterrupt` macro not backported until Arduino IDE v.1.0.6
  attachInterrupt(digitalPinToInterrupt(ENC_PIN_A), isr_wrapper, CHANGE);
  */
  setUpInterrupt();

  // success
  return true;
}

bool Knitter::startTest(Machine_t machineType) {
  bool success = false;
  if (s_init == m_opState || s_ready == m_opState) {
    m_opState = s_test;
    m_machineType = machineType;
    GlobalTester::setUp();
    success = true;
  }
  return success;
}

uint8_t Knitter::getStartOffset(const Direction_t direction) {
  if ((direction == NoDirection) || (direction >= NUM_DIRECTIONS) ||
      (m_carriage == NoCarriage) || (m_carriage >= NUM_CARRIAGES) ||
      (m_machineType == NoMachine) || (m_machineType >= NUM_MACHINES)) {
    // TODO(TP): return error state
    return 0U;
  }
  return START_OFFSET[m_machineType][direction][m_carriage];
}

Machine_t Knitter::getMachineType() {
  return m_machineType;
}

void Knitter::setSolenoids(uint16_t state) {
  m_solenoids.setSolenoids(state);
}

bool Knitter::setNextLine(uint8_t lineNumber) {
  bool success = false;
  if (m_lineRequested) {
    // FIXME: Is there even a need for a new line?
    if (lineNumber == m_currentLineNumber) {
      m_lineRequested = false;
      GlobalBeeper::finishedLine();
      success = true;
    } else {
      // line numbers didn't match -> request again
      reqLine(m_currentLineNumber);
    }
  }
  return success;
}

void Knitter::setLastLine() {
  // lastLineFlag is evaluated in s_operate
  m_lastLineFlag = true;
}

void Knitter::setMachineType(Machine_t machineType) {
  m_machineType = machineType;
}

void Knitter::setState(OpState_t state) {
  m_opState = state;
}

// private methods

void Knitter::state_init() {
#ifdef DBG_NOMACHINE
  bool state = digitalRead(DBG_BTN_PIN);

  // TODO(who?): check if debounce is needed
  if (m_prevState && !state) {
#else
  // machine is initialized when left hall sensor is passed in Right direction
  if (Right == m_direction && Left == m_hallActive) {
#endif // DBG_NOMACHINE
    m_opState = s_ready;
    m_solenoids.setSolenoids(SOLENOIDS_BITMASK);
    indState(true);
  }

#ifdef DBG_NOMACHINE
  m_prevState = state;
#endif
}

void Knitter::state_ready() {
  digitalWrite(LED_PIN_A, 0); // green LED off
  // This state is left when the startOperation() method
  // is called successfully by fsm()
}

void Knitter::state_operate() {
  digitalWrite(LED_PIN_A, 1); // green LED on

  if (m_firstRun) {
    m_firstRun = false;
    // TODO(who?): optimize delay for various Arduino models
    delay(START_OPERATION_DELAY);
    GlobalBeeper::finishedLine();
    reqLine(++m_currentLineNumber);
  }

#ifdef DBG_NOMACHINE
  bool state = digitalRead(DBG_BTN_PIN);

  // TODO(who?): check if debounce is needed
  if (m_prevState && !state) {
    if (!m_lineRequested) {
      reqLine(++m_currentLineNumber);
    }
  }
  m_prevState = state;
#else
  if (m_sOldPosition != m_position) {
    // only act if there is an actual change of position
    // store current Encoder position for next call of this function
    m_sOldPosition = m_position;

    if (m_continuousReportingEnabled) {
      // send current position to GUI
      indState(true);
    }

    if (!calculatePixelAndSolenoid()) {
      // no valid/useful position calculated
      return;
    }

    // m_machineType has been validated
    if ((m_pixelToSet >= m_startNeedle - END_OF_LINE_OFFSET_L[m_machineType]) &&
        (m_pixelToSet <= m_stopNeedle + END_OF_LINE_OFFSET_R[m_machineType])) {

      if ((m_pixelToSet >= m_startNeedle) && (m_pixelToSet <= m_stopNeedle)) {
        // when inside the active needles
        if (m_machineType == Kh270) {
          digitalWrite(LED_PIN_B, 1); // yellow LED on
        }
        m_workedOnLine = true;
      }

      // find the right byte from the currentLine array,
      // then read the appropriate Pixel(/Bit) for the current needle to set
      uint8_t currentByte = m_pixelToSet / 8U;
      bool pixelValue =
          bitRead(m_lineBuffer[currentByte], m_pixelToSet - (8U * currentByte));
      // write Pixel state to the appropriate needle
      m_solenoids.setSolenoid(m_solenoidToSet, pixelValue);
    } else {
      // outside of the active needles
      if (m_machineType == Kh270) {
        digitalWrite(LED_PIN_B, 0); // yellow LED off
      }

      // reset solenoids when out of range
      m_solenoids.setSolenoid(m_solenoidToSet, true);

      if (m_workedOnLine) {
        // already worked on the current line -> finished the line
        m_workedOnLine = false;

        if (!m_lineRequested && !m_lastLineFlag) {
          // request new line from host
          reqLine(++m_currentLineNumber);
        } else if (m_lastLineFlag) {
          stopOperation();
        }
      }
    }
  }
#endif // DBG_NOMACHINE
}

void Knitter::state_test() {
  if (m_sOldPosition != m_position) {
    // only act if there is an actual change of position
    // store current encoder position for next call of this function
    m_sOldPosition = m_position;
    calculatePixelAndSolenoid();
    indState();
  }
  GlobalTester::loop();
  if (GlobalTester::getQuitFlag()) {
    m_opState = s_ready;
  }
}

bool Knitter::calculatePixelAndSolenoid() {
  uint8_t startOffset = 0;
  bool success = true;

  switch (m_direction) {
  // calculate the solenoid and pixel to be set
  // implemented according to machine manual
  // magic numbers from machine manual
  case Right:
    startOffset = getStartOffset(Left);
    if (m_position >= startOffset) {
      m_pixelToSet = m_position - startOffset;

      if (m_machineType == Kh270) {
        // TODO(who?): check
        m_solenoidToSet = (m_position % 12) + 3;
      } else {
        if (Regular == m_beltshift) {
          m_solenoidToSet = m_position % SOLENOIDS_NUM;
        } else if (Shifted == m_beltshift) {
          m_solenoidToSet = (m_position - HALF_SOLENOIDS_NUM) % SOLENOIDS_NUM;
        }
        if (Lace == m_carriage) {
          m_pixelToSet = m_pixelToSet + HALF_SOLENOIDS_NUM;
        }
      }
    } else {
      success = false;
    }
    break;

  case Left:
    startOffset = getStartOffset(Right);
    if (m_position <= (END_RIGHT[m_machineType] - startOffset)) {
      m_pixelToSet = m_position - startOffset;

      if (m_machineType == Kh270) {
        // TODO(who?): check
        m_solenoidToSet = ((m_position + 6) % 12) + 3;
      } else {
        if (Regular == m_beltshift) {
          m_solenoidToSet = (m_position + HALF_SOLENOIDS_NUM) % SOLENOIDS_NUM;
        } else if (Shifted == m_beltshift) {
          m_solenoidToSet = m_position % SOLENOIDS_NUM;
        }
        if (Lace == m_carriage) {
          m_pixelToSet = m_pixelToSet - SOLENOIDS_NUM;
        }
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

void Knitter::reqLine(const uint8_t lineNumber) {
  uint8_t payload[REQLINE_LEN] = {
      reqLine_msgid,
      lineNumber,
  };
  GlobalCom::send(static_cast<uint8_t *>(payload), REQLINE_LEN);

  m_lineRequested = true;
}

void Knitter::indState(const bool initState) {
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
  GlobalCom::send(static_cast<uint8_t *>(payload), INDSTATE_LEN);
}

void Knitter::stopOperation() {
  GlobalBeeper::endWork();
  m_opState = s_ready;

  m_solenoids.setSolenoids(SOLENOIDS_BITMASK);
  GlobalBeeper::finishedLine();

  // detaching ENC_PIN_A, Interrupt #0
  /*
  // `digitalPinToInterrupt` macro not backported until Arduino !DE v.1.0.6
  detachInterrupt(digitalPinToInterrupt(ENC_PIN_A));
  */
  detachInterrupt(0);
}

OpState_t Knitter::getState() const {
  return m_opState;
}
