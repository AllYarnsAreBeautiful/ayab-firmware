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

  // FIXME(TP): should this go in `main()`?
  GlobalSolenoids::init();

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
  m_lastHall = NoDirection;
  m_position = 0U;
  m_hallActive = NoDirection;
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
 * \brief Initialize machine type.
 * \param machineType Machine type.
 * \return Error code (0 = success, other values = error).
 */
Err_t Knitter::initMachine(Machine_t machineType) {
  if (GlobalFsm::getState() != OpState::wait_for_machine) {
    return ERR_WRONG_MACHINE_STATE;
  }
  if (machineType == NoMachine) {
    return ERR_NO_MACHINE_TYPE;
  }
  if (machineType >= NUM_MACHINES) {
    return ERR_MACHINE_TYPE_INVALID;
  }

  m_machineType = machineType;

  GlobalEncoders::init(machineType);
  GlobalFsm::setState(OpState::init);

  // Now that we have enough start state, we can set up interrupts
  setUpInterrupt();

  return SUCCESS;
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
    return ERR_WRONG_MACHINE_STATE;
  }
  if (pattern_start == nullptr) {
    return ERR_NULL_POINTER_ARGUMENT;
  }
  if (startNeedle >= stopNeedle || stopNeedle >= NUM_NEEDLES[m_machineType]) {
    return ERR_NEEDLE_VALUE_INVALID;
  }

  // record argument values
  m_startNeedle = startNeedle;
  m_stopNeedle = stopNeedle;
  m_lineBuffer = pattern_start;
  m_continuousReportingEnabled = continuousReportingEnabled;

  // reset variables to start conditions
  m_currentLineNumber = UINT8_MAX; // because counter will
                                   // be incremented before request
  m_lineRequested = false;
  m_lastLineFlag = false;

  // proceed to next state
  GlobalFsm::setState(OpState::knit);
  GlobalBeeper::ready();

  // success
  return SUCCESS;
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
    indState(UNSPECIFIED_FAILURE);
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
  if (m_hallActive != NoDirection) {
    m_lastHall = m_hallActive;
  }

  bool passedLeft = Right == m_direction and Left == m_lastHall and
        m_position > (END_LEFT[m_machineType] + END_OFFSET[m_machineType] + GARTER_SLOP);
  bool passedRight = Left == m_direction and Right == m_lastHall and
        m_position < (END_RIGHT[m_machineType] - END_OFFSET[m_machineType] - GARTER_SLOP);
  // Machine is initialized when left Hall sensor is passed in Right direction
  // New feature (August 2020): the machine is also initialized
  // when the right Hall sensor is passed in Left direction.
  if (passedLeft || passedRight) {

#endif // DBG_NOMACHINE
    GlobalSolenoids::setSolenoids(SOLENOIDS_BITMASK);
    indState(SUCCESS);
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
    // TODO(who?): optimize delay for various Arduino models
    delay(START_KNITTING_DELAY);
    GlobalBeeper::finishedLine();
    reqLine(++m_currentLineNumber);
  }

#ifdef DBG_NOMACHINE
  // TODO(who?): check if debounce is needed
  bool state = digitalRead(DBG_BTN_PIN);

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

  // Desktop software is setting flanking needles so we need to set
  // these even outside of the working needles.
  // find the right byte from the currentLine array,
  // then read the appropriate Pixel(/Bit) for the current needle to set
  uint8_t currentByte = m_pixelToSet >> 3;
  bool pixelValue =
      bitRead(m_lineBuffer[currentByte], m_pixelToSet & 0x07);
  // write Pixel state to the appropriate needle
  GlobalSolenoids::setSolenoid(m_solenoidToSet, pixelValue);

  if ((m_pixelToSet >= m_startNeedle) && (m_pixelToSet <= m_stopNeedle)) {
    m_workedOnLine = true;
  }

  if ((m_pixelToSet < m_startNeedle - END_OF_LINE_OFFSET_L[m_machineType]) ||
      (m_pixelToSet > m_stopNeedle + END_OF_LINE_OFFSET_R[m_machineType])) {
    // outside of the active needles

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
  if ((direction == NoDirection) || (direction >= NUM_DIRECTIONS) ||
      (m_carriage == NoCarriage) || (m_carriage >= NUM_CARRIAGES) ||
      (m_machineType == NoMachine) || (m_machineType >= NUM_MACHINES)) {
    // TODO(TP): return error state?
    return 0U;
  }
  return START_OFFSET[m_machineType][direction][m_carriage];
}

/*!
 * \brief Set line number of next row to be knitted.
 * \param lineNumber Line number (0-indexed and modulo 256).
 */
bool Knitter::setNextLine(uint8_t lineNumber) {
  bool success = false;
  if (m_lineRequested) {
    // FIXME: Is there even a need for a new line?
    if (lineNumber == m_currentLineNumber) {
      m_lineRequested = false;

      // Beeper is causing problems with flanking needles on the 270
      if (m_machineType != Kh270) {
        GlobalBeeper::finishedLine();
      }
      success = true;
    } else {
      // line numbers didn't match -> request again
      reqLine(m_currentLineNumber);
    }
  }
  return success;
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
  GlobalCom::send_reqLine(lineNumber, SUCCESS);
  m_lineRequested = true;
}

/*!
 * \brief Calculate the solenoid and pixel to be set.
 * \param `true` if successful, `false` otherwise.
 */
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

      if (BeltShift::Regular == m_beltShift || m_machineType == Kh270) {
        m_solenoidToSet = m_position % SOLENOIDS_NUM[m_machineType];
      } else if (BeltShift::Shifted == m_beltShift) {
        m_solenoidToSet = (m_position - HALF_SOLENOIDS_NUM[m_machineType]) % SOLENOIDS_NUM[m_machineType];
      }
      if (Lace == m_carriage) {
        m_pixelToSet = m_pixelToSet + HALF_SOLENOIDS_NUM[m_machineType];
      }

      // The 270 has 12 solenoids but they get shifted over 3 bits
      if (m_machineType == Kh270) {
        m_solenoidToSet = m_solenoidToSet + 3;
      }
    } else {
      success = false;
    }
    break;

  case Left:
    startOffset = getStartOffset(Right);
    if (m_position <= (END_RIGHT[m_machineType] - startOffset)) {
      m_pixelToSet = m_position - startOffset;

      if (BeltShift::Regular == m_beltShift || m_machineType == Kh270) {
        m_solenoidToSet = (m_position + HALF_SOLENOIDS_NUM[m_machineType]) % SOLENOIDS_NUM[m_machineType];
      } else if (BeltShift::Shifted == m_beltShift) {
        m_solenoidToSet = m_position % SOLENOIDS_NUM[m_machineType];
      }
      if (Lace == m_carriage) {
        m_pixelToSet = m_pixelToSet - SOLENOIDS_NUM[m_machineType];
      }

      // The 270 has 12 solenoids but they get shifted over 3 bits
      if (m_machineType == Kh270) {
        m_solenoidToSet = m_solenoidToSet + 3;
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

/*!
 * \brief Finish knitting procedure.
 */
const void Knitter::stopKnitting() {
  GlobalBeeper::endWork();
  GlobalFsm::setState(OpState::ready);

  GlobalSolenoids::setSolenoids(SOLENOIDS_BITMASK);
  GlobalBeeper::finishedLine();

  // detaching ENC_PIN_A, Interrupt #0
  /*
  // `digitalPinToInterrupt` macro not backported until Arduino !DE v.1.0.6
  detachInterrupt(digitalPinToInterrupt(ENC_PIN_A));
  */
  // detachInterrupt(0);
}
