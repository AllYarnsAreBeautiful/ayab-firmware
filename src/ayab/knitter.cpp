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
 *    Modified Work Copyright 2020-3 Sturla Lange, Tom Price
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

  GlobalSolenoids::init();

  // explicitly initialize members

  // job parameters
  m_machineType = Machine_t::NoMachine;
  m_startNeedle = 0U;
  m_stopNeedle = 0U;
  m_lineBuffer = nullptr;
  m_continuousReportingEnabled = false;

  m_lineRequested = false;
  m_currentLineNumber = 0U;
  m_lastLineFlag = false;
  m_sOldPosition = 0U;
  m_firstRun = true;
  m_lastHall = Direction_t::NoDirection;
  m_position = 0U;
  m_hallActive = Direction_t::NoDirection;
  m_pixelToSet = 0;
#ifdef DBG_NOMACHINE
  m_prevState = false;
#endif
}

/*!
 * \brief Initialize interrupt service routine for Knitter object.
 */
void Knitter::setUpInterrupt() {
  // (re-)attach ENC_PIN_A(=2), interrupt #0
  detachInterrupt(digitalPinToInterrupt(ENC_PIN_A));
#ifndef AYAB_TESTS
  // Attaching ENC_PIN_A, Interrupt #0
  // This interrupt cannot be enabled until
  // the machine type has been validated.
  attachInterrupt(digitalPinToInterrupt(ENC_PIN_A), GlobalKnitter::isr, CHANGE);
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
 * \brief Initialize machine type.
 * \param machineType Machine type.
 * \return Error code (0 = success, other values = error).
 */
Err_t Knitter::initMachine(Machine_t machineType) {
  if (GlobalFsm::getState() != OpState::wait_for_machine) {
    return ErrorCode::wrong_machine_state;
  }
  if (machineType == Machine_t::NoMachine) {
    return ErrorCode::no_machine_type;
  }
  m_machineType = machineType;

  GlobalEncoders::init(machineType);
  GlobalFsm::setState(OpState::init);

  // Now that we have enough start state, we can set up interrupts
  setUpInterrupt();

  return ErrorCode::success;
}

/*!
 * \brief Enter `OpState::knit` machine state.
 * \param startNeedle Position of first needle in the pattern.
 * \param stopNeedle Position of last needle in the pattern.
 * \param patternStart Pointer to buffer containing pattern data.
 * \param continuousReportingEnabled Flag variable indicating whether the device continuously reports its status to the host.
 * \return Error code (0 = success, other values = error).
 */
Err_t Knitter::startKnitting(uint8_t startNeedle,
                             uint8_t stopNeedle, uint8_t *pattern_start,
                             bool continuousReportingEnabled) {
  if (GlobalFsm::getState() != OpState::ready) {
    return ErrorCode::wrong_machine_state;
  }
  if (pattern_start == nullptr) {
    return ErrorCode::null_pointer_argument;
  }
  if ((startNeedle >= stopNeedle) || (stopNeedle >= NUM_NEEDLES[static_cast<uint8_t>(m_machineType)])) {
    return ErrorCode::needle_value_invalid;
  }

  // record argument values
  m_startNeedle = startNeedle;
  m_stopNeedle = stopNeedle;
  m_lineBuffer = pattern_start;
  m_continuousReportingEnabled = continuousReportingEnabled;

  // reset variables to start conditions
  m_currentLineNumber = UINT8_MAX; // because counter will
                                   // be incremented before request
  m_currentLineDirection = Direction::NoDirection;
  m_lineRequested = false;
  m_lastLineFlag = false;

  // proceed to next state
  GlobalFsm::setState(OpState::knit);
  GlobalBeeper::ready();

  // success
  return ErrorCode::success;
}

/*!
 * \brief Record current encoder position.
 *
 * Used in hardware test procedure.
 */
void Knitter::encodePosition() {
  if (m_sOldPosition != m_position) {
    // only act if there is an actual change of position
    // store current encoder position for next call of this function
    m_sOldPosition = m_position;
    calculatePixelAndSolenoid();
    indState(ErrorCode::unspecified_failure);
  }
}

/*!
 * \brief Assess whether the Finite State Machine is ready to move from state `OpState::init` to `OpState::ready`.
 * \return `true` if ready to move from state `OpState::init` to `OpState::ready`, false otherwise.
 */
bool Knitter::isReady() {
#ifdef DBG_NOMACHINE
  // TODO(who?): check if debounce is needed
  bool state = digitalRead(DBG_BTN_PIN);

  if (m_prevState && !state) {
#else
  // In order to support the garter carriage, we need to wait and see if there
  // will be a second magnet passing the sensor.
  // Keep track of the last seen hall sensor because we may be making a decision
  // after it passes.
  if (m_hallActive != Direction_t::NoDirection) {
    m_lastHall = m_hallActive;
  }

  bool passedLeft = (Direction_t::Right == m_direction) && (Direction_t::Left == m_lastHall) &&
        (m_position > (END_LEFT_PLUS_OFFSET[static_cast<uint8_t>(m_machineType)] + GARTER_SLOP));
  bool passedRight = (Direction_t::Left == m_direction) && (Direction_t::Right == m_lastHall) &&
        (m_position < (END_RIGHT_MINUS_OFFSET[static_cast<uint8_t>(m_machineType)] - GARTER_SLOP));
  // Machine is initialized when left Hall sensor is passed in Right direction
  // New feature (August 2020): the machine is also initialized
  // when the right Hall sensor is passed in Left direction.
  if (passedLeft || passedRight) {

#endif // DBG_NOMACHINE
    GlobalSolenoids::setSolenoids(SOLENOIDS_BITMASK);
    indState(ErrorCode::success);
    return true; // move to `OpState::ready`
  }

#ifdef DBG_NOMACHINE
  m_prevState = state;
#endif
  return false; // stay in `OpState::init`
}

/*!
 * \brief Function that is repeatedly called during state `OpState::knit`
 */
void Knitter::knit() {
  if (m_firstRun) {
    m_firstRun = false;
    GlobalBeeper::finishedLine();
    ++m_currentLineNumber;
    // first line direction is based on which Hall sensor was passed
    m_currentLineDirection =
        m_lastHall == Direction::Left ? Direction::Right : Direction::Left;
    reqLine(m_currentLineNumber);
  }

#ifdef DBG_NOMACHINE
  // TODO(who?): check if debounce is needed
  bool state = digitalRead(DBG_BTN_PIN);

  if (m_prevState && !state && !m_lineRequested) {
    ++m_currentLineNumber;
    reqLine(m_currentLineNumber);
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
    indState(ErrorCode::success);
  }

  if (!calculatePixelAndSolenoid()) {
    // no valid/useful position calculated
    return;
  }

  // Desktop software is setting flanking needles so we need to set
  // these even outside of the working needles.
  // find the right byte from the currentLine array,
  // then read the appropriate Pixel(/Bit) for the current needle to set
  uint8_t currentByte = m_pixelToSet >> 3;
  bool pixelValue =
      bitRead(m_lineBuffer[currentByte], m_pixelToSet & 0x07);
  // write Pixel state to the appropriate needle
  GlobalSolenoids::setSolenoid(m_solenoidToSet, pixelValue);

  if (isLineFinished()) {
    if (!m_lineRequested && !m_lastLineFlag) {
      // flip line direction
      m_currentLineDirection = m_currentLineDirection == Direction::Left
                                   ? Direction::Right
                                   : Direction::Left;
      // request new line from host
      ++m_currentLineNumber;
      reqLine(m_currentLineNumber);
    } else if (m_lastLineFlag) {
      stopKnitting();
    }
  }
#endif // DBG_NOMACHINE
}

/*!
 * \brief Send `indState` message.
 * \param error Error state (0 = success, other values = error).
 */
void Knitter::indState(Err_t error) {
  GlobalCom::send_indState(m_carriage, m_position, error);
}

/*!
 * \brief Get knitting machine type.
 * \return Machine type.
 */
Machine_t Knitter::getMachineType() {
  return m_machineType;
}

/*!
 * \brief Get start offset.
 * \return Start offset, or 0 if unobtainable.
 */
uint8_t Knitter::getStartOffset(const Direction_t direction) {
  if ((direction == Direction_t::NoDirection) ||
      (m_carriage == Carriage_t::NoCarriage) ||
      (m_machineType == Machine_t::NoMachine)) {
    return 0U;
  }
  return START_OFFSET[static_cast<uint8_t>(m_machineType)][static_cast<uint8_t>(direction)][static_cast<uint8_t>(m_carriage)];
}

/*!
 * \brief Set line number of next row to be knitted.
 * \param lineNumber Line number (0-indexed and modulo 256).
 * \return `true` if successful, `false` otherwise.
 */
bool Knitter::setNextLine(uint8_t lineNumber) {
  if (m_lineRequested) {
    // Is there even a need for a new line?
    if (lineNumber == m_currentLineNumber) {
      m_lineRequested = false;
      GlobalBeeper::finishedLine();
      return true;
    } else {
      // line numbers didn't match -> request again
      reqLine(m_currentLineNumber);
    }
  }
  return false;
}

/*!
 * \brief Get value of last line flag.
 * \param `true` if current line is the last line in the pattern, `false` otherwise.
 */
void Knitter::setLastLine() {
  m_lastLineFlag = true;
}

/*!
 * \brief Set machine type.
 * \param Machine type.
 */
void Knitter::setMachineType(Machine_t machineType) {
  m_machineType = machineType;
}

// private methods

/*!
 * \brief Send `reqLine` message.
 * \param lineNumber Line number requested.
 */
void Knitter::reqLine(uint8_t lineNumber) {
  GlobalCom::send_reqLine(lineNumber, ErrorCode::success);
  m_lineRequested = true;
}

/*!
 * \brief Calculate the solenoid and pixel to be set.
 * \return `true` if successful, `false` otherwise.
 */
bool Knitter::calculatePixelAndSolenoid() {
  uint8_t startOffset = 0;
  uint8_t laceOffset = 0;

  switch (m_currentLineDirection) {
  // calculate the solenoid and pixel to be set
  // implemented according to machine manual
  // magic numbers from machine manual
  case Direction_t::Right:
    startOffset = getStartOffset(Direction_t::Left);

    // We have to start setting pixels earlier when the lace carriage is selected because we shift
    // the lace pixel selection up HALF_SOLENOIDS_NUM in this direction. Doesn't matter going back 
    // the other way.
    if (Carriage_t::Lace == m_carriage) {
      laceOffset = HALF_SOLENOIDS_NUM[static_cast<uint8_t>(m_machineType)];
    }

    if (m_position >= startOffset - laceOffset) {
      m_pixelToSet = m_position - startOffset;

      if ((BeltShift::Regular == m_beltShift) || (m_machineType == Machine_t::Kh270)) {
        m_solenoidToSet = m_position % SOLENOIDS_NUM[static_cast<uint8_t>(m_machineType)];
      } else if (BeltShift::Shifted == m_beltShift) {
        m_solenoidToSet = (m_position - HALF_SOLENOIDS_NUM[static_cast<uint8_t>(m_machineType)]) % SOLENOIDS_NUM[static_cast<uint8_t>(m_machineType)];
      }
      if (Carriage_t::Lace == m_carriage) {
        m_pixelToSet = m_pixelToSet + HALF_SOLENOIDS_NUM[static_cast<uint8_t>(m_machineType)];
      }
    } else {
      return false;
    }
    break;

  case Direction_t::Left:
    startOffset = getStartOffset(Direction_t::Right);
    if (m_position <= (END_RIGHT[static_cast<uint8_t>(m_machineType)] - startOffset)) {
      m_pixelToSet = m_position - startOffset;

      if ((BeltShift::Regular == m_beltShift) || (m_machineType == Machine_t::Kh270)) {
        m_solenoidToSet = (m_position + HALF_SOLENOIDS_NUM[static_cast<uint8_t>(m_machineType)]) % SOLENOIDS_NUM[static_cast<uint8_t>(m_machineType)];
      } else if (BeltShift::Shifted == m_beltShift) {
        m_solenoidToSet = m_position % SOLENOIDS_NUM[static_cast<uint8_t>(m_machineType)];
      }
      if (Carriage_t::Lace == m_carriage) {
        m_pixelToSet = m_pixelToSet - SOLENOIDS_NUM[static_cast<uint8_t>(m_machineType)];
      }
    } else {
      return false;
    }
    break;

  default:
    return false;
  }

  // The 270 has 12 solenoids but they get shifted over 3 bits
  if (m_machineType == Machine_t::Kh270) {
    m_solenoidToSet = m_solenoidToSet + 3;
  }
  return true;
}

/*!
 * \brief Assess whether the current line is done being knitted, based on the
 * position of the carriage.
 * \return `true` if the line is finished, `false` * otherwise.
 */
bool Knitter::isLineFinished() {
  // Compute the position of both "needle selectors" as they both need to clear
  // the last needle before the carriage can safely turn around.
  // Using `int` here as the results may be negative.
  int leftSelectorPosition = m_position - getStartOffset(Direction::Left);
  int rightSelectorPosition = m_position - getStartOffset(Direction::Right);

  // The limits we are testing against are the working needles plus a safety
  // margin
  int leftLimit =
      m_startNeedle - END_OF_LINE_OFFSET_L[static_cast<uint8_t>(m_machineType)];
  int rightLimit =
      m_stopNeedle + END_OF_LINE_OFFSET_R[static_cast<uint8_t>(m_machineType)];

  // If going left, both selectors must have cleared the left limit before we
  // consider the row done; if going right, both selectors must have cleared the
  // right limit.
  return ((m_currentLineDirection == Direction::Left &&
           leftSelectorPosition < leftLimit &&
           rightSelectorPosition < leftLimit) ||
          (m_currentLineDirection == Direction::Right &&
           leftSelectorPosition > rightLimit &&
           rightSelectorPosition > rightLimit));
}

/*!
 * \brief Finish knitting procedure.
 */
void Knitter::stopKnitting() const {
  GlobalBeeper::endWork();
  GlobalFsm::setState(OpState::init);

  GlobalSolenoids::setSolenoids(SOLENOIDS_BITMASK);
  GlobalBeeper::finishedLine();

  // detaching ENC_PIN_A, Interrupt #0
  /* detachInterrupt(digitalPinToInterrupt(ENC_PIN_A)); */
}
