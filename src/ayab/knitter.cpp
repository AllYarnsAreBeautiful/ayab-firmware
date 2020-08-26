/*!
 * \file knitter.cpp
 * \brief Class containing methods for the finite state machine
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

#include "board.h"
#include <Arduino.h>

#include "beeper.h"
#include "com.h"
#include "encoders.h"
#include "fsm.h"
#include "knitter.h"
#include "tester.h"

#ifdef CLANG_TIDY
// clang-tidy doesn't find these macros for some reason,
// no problem when building or testing though.
constexpr uint8_t UINT8_MAX = 0xFFU;
constexpr uint16_t UINT16_MAX = 0xFFFFU;
#endif

/*!
 * \brief Initialize Knitter object.
 *
 * Initialize the solenoids as well as pins and interrupts.
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

  // FIXME(TP): should this go in `main()`?
  GlobalSolenoids::init();

  setUpInterrupt();

  // explicitly initialize members

  // job parameters
  m_machineType = NoMachine;
  m_startNeedle = 0U;
  m_stopNeedle = 0U;
  m_lineBuffer = nullptr;
  m_continuousReportingEnabled = false;

  m_lineRequested = false;
  m_currentLineNumber = 0U;
  m_lastLineFlag = false;
  m_sOldPosition = 0U;
  m_firstRun = true;
  m_workedOnLine = false;
#ifdef DBG_NOMACHINE
  m_prevState = false;
#endif
}

void Knitter::setUpInterrupt() {
  // (re-)attach ENC_PIN_A(=2), interrupt #0
  detachInterrupt(0);
#ifndef AYAB_TESTS
  // Attaching ENC_PIN_A, Interrupt #0
  // This interrupt cannot be enabled until
  // the machine type has been validated.
  /*
  // `digitalPinToInterrupt` macro not backported until Arduino IDE v.1.0.6
  attachInterrupt(digitalPinToInterrupt(ENC_PIN_A), isr_wrapper, CHANGE);
  */
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
  GlobalEncoders::encA_interrupt();
  m_position = GlobalEncoders::getPosition();
  m_direction = GlobalEncoders::getDirection();
  m_hallActive = GlobalEncoders::getHallActive();
  m_beltShift = GlobalEncoders::getBeltShift();
  m_carriage = GlobalEncoders::getCarriage();
}

/*!
 * \brief Enter knit state.
 *
 * Note (August 2020): the return value of this function has changed.
 * Previously, it returned `true` for success and `false` for failure.
 * Now, it returns `0` for success and an informative error code otherwise.
 */
Err_t Knitter::startKnitting(Machine_t machineType, uint8_t startNeedle,
                             uint8_t stopNeedle, uint8_t *pattern_start,
                             bool continuousReportingEnabled) {
  if (GlobalFsm::getState() != s_ready) {
    return WRONG_MACHINE_STATE;
  }
  if (machineType == NoMachine) {
    return NO_MACHINE_TYPE;
  }
  if (machineType >= NUM_MACHINES) {
    return MACHINE_TYPE_INVALID;
  }
  if (pattern_start == nullptr) {
    return NULL_POINTER_ARGUMENT;
  }
  if (startNeedle >= stopNeedle || stopNeedle >= NUM_NEEDLES[machineType]) {
    return NEEDLE_VALUE_INVALID;
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
  GlobalEncoders::init(machineType);

  // proceed to next state
  GlobalFsm::setState(s_knit);
  GlobalBeeper::ready();

  // success
  return SUCCESS;
}

// used in hardware test loop
void Knitter::encodePosition() {
  if (m_sOldPosition != m_position) {
    // only act if there is an actual change of position
    // store current encoder position for next call of this function
    m_sOldPosition = m_position;
    calculatePixelAndSolenoid();
    indState(UNSPECIFIED_FAILURE);
  }
}

// if this function returns true then
// the FSM will move from state `s_init` to `s_ready`
bool Knitter::isReady() {
#ifdef DBG_NOMACHINE
  bool state = digitalRead(DBG_BTN_PIN);

  // TODO(who?): check if debounce is needed
  if (m_prevState && !state) {
#else
  // Machine is initialized when left Hall sensor is passed in Right direction
  // New feature (August 2020): the machine is also initialized
  // when the right Hall sensor is passed in Left direction.
  if ((Right == m_direction and Left == m_hallActive) or
      (Left == m_direction and Right == m_hallActive)) {

#endif // DBG_NOMACHINE
    GlobalSolenoids::setSolenoids(SOLENOIDS_BITMASK);
    indState(SUCCESS);
    return true; // move to `s_ready`
  }

#ifdef DBG_NOMACHINE
  m_prevState = state;
#endif
  return false; // stay in `s_init`
}

void Knitter::knit() {
  if (m_firstRun) {
    m_firstRun = false;
    // TODO(who?): optimize delay for various Arduino models
    delay(START_KNITTING_DELAY);
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
  // only act if there is an actual change of position
  if (m_sOldPosition == m_position) {
    return;
  }

  // store current carriage position for next call of this function
  m_sOldPosition = m_position;

  if (m_continuousReportingEnabled) {
    // send current position to GUI
    indState(SUCCESS);
  }

  if (!calculatePixelAndSolenoid()) {
    // no valid/useful position calculated
    return;
  }

  // `m_machineType` has been validated - no need to check
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
    GlobalSolenoids::setSolenoid(m_solenoidToSet, pixelValue);
  } else {
    // outside of the active needles
    if (m_machineType == Kh270) {
      digitalWrite(LED_PIN_B, 0); // yellow LED off
    }

    // reset solenoids when out of range
    GlobalSolenoids::setSolenoid(m_solenoidToSet, true);

    if (m_workedOnLine) {
      // already worked on the current line -> finished the line
      m_workedOnLine = false;

      if (!m_lineRequested && !m_lastLineFlag) {
        // request new line from host
        reqLine(++m_currentLineNumber);
      } else if (m_lastLineFlag) {
        stopKnitting();
      }
    }
  }
#endif // DBG_NOMACHINE
}

void Knitter::indState(Err_t error) {
  GlobalCom::send_indState(m_carriage, m_position, error);
}

Machine_t Knitter::getMachineType() {
  return m_machineType;
}

uint8_t Knitter::getStartOffset(const Direction_t direction) {
  if ((direction == NoDirection) || (direction >= NUM_DIRECTIONS) ||
      (m_carriage == NoCarriage) || (m_carriage >= NUM_CARRIAGES) ||
      (m_machineType == NoMachine) || (m_machineType >= NUM_MACHINES)) {
    // TODO(TP): return error state?
    return 0U;
  }
  return START_OFFSET[m_machineType][direction][m_carriage];
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

// private methods

void Knitter::reqLine(uint8_t lineNumber) {
  GlobalCom::send_reqLine(lineNumber, SUCCESS);
  m_lineRequested = true;
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
        if (Regular == m_beltShift) {
          m_solenoidToSet = m_position % SOLENOIDS_NUM;
        } else if (Shifted == m_beltShift) {
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
        if (Regular == m_beltShift) {
          m_solenoidToSet = (m_position + HALF_SOLENOIDS_NUM) % SOLENOIDS_NUM;
        } else if (Shifted == m_beltShift) {
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

void Knitter::stopKnitting() {
  GlobalBeeper::endWork();
  GlobalFsm::setState(s_ready);

  GlobalSolenoids::setSolenoids(SOLENOIDS_BITMASK);
  GlobalBeeper::finishedLine();

  // detaching ENC_PIN_A, Interrupt #0
  /*
  // `digitalPinToInterrupt` macro not backported until Arduino !DE v.1.0.6
  detachInterrupt(digitalPinToInterrupt(ENC_PIN_A));
  */
  // detachInterrupt(0);
}
