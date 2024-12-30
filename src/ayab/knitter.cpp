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
  m_workedOnLine = false;
  m_lastHall = Direction_t::NoDirection;
  m_position = 0U;
  m_carriage = Carriage_t::NoCarriage;
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
  // Machine is initialized when the left Hall sensor is passed in Right
  // direction, or the right Hall sensor is passed in Left direction. Or, as
  // soon as we have detected a Garter carriage, because in that case we may
  // need to start setting solenoids before the carriage center has crossed the
  // turn mark.
  if (passedLeft || passedRight || m_carriage == Carriage_t::Garter) {

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
    // This will only happen if there's an error
    GlobalBeeper::error();
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

  if (((m_pixelToSet < m_startNeedle - END_OF_LINE_OFFSET_L[static_cast<uint8_t>(m_machineType)]) ||
       (m_pixelToSet > m_stopNeedle + END_OF_LINE_OFFSET_R[static_cast<uint8_t>(m_machineType)])) &&
      m_workedOnLine) {
    // outside of the active needles and
    // already worked on the current line -> finished the line
    m_workedOnLine = false;

    if (!m_lineRequested && !m_lastLineFlag) {
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

  bool beltShift = BeltShift_t::Shifted == m_beltShift;

  // 270 Doesn't care about belt shift
  if (Machine_t::Kh270 == m_machineType) {
    beltShift = false;
  }

  // 270 needs additional start offsets because of it's wierdness
  uint8_t bulkyOffset = 0;

  switch (m_direction) {
  // calculate the solenoid and pixel to be set
  // implemented according to machine manual
  // magic numbers from machine manual
  case Direction_t::Right:
    startOffset = getStartOffset(Direction_t::Left);

    // The Lace carriage is special
    // See page 7 of the 930 service manual https://mkmanuals.com/downloadable/download/sample/sample_id/27/
    if (Carriage_t::Lace == m_carriage) {
      beltShift = !beltShift;
    }

    // Page 6 of the 270 service manual: https://mostlyknittingmachines.weebly.com/uploads/8/4/6/7/846749/brother_kh270_service_manual.pdf
    // Pixel 0 needs to be written into solenoid 4 (indexed from 0) L -> R
    if (Machine_t::Kh270 == m_machineType) {
      bulkyOffset = 4;
    }

    break;
  case Direction_t::Left:
    startOffset = getStartOffset(Direction_t::Right);

    // Page 6 of the 270 service manual: https://mostlyknittingmachines.weebly.com/uploads/8/4/6/7/846749/brother_kh270_service_manual.pdf
    // Pixel 0 needs to be written into solenoid 10 (indexed from 0) R -> L
    if (Machine_t::Kh270 == m_machineType) {
      bulkyOffset = 10;
    }

    break;
  default:
    return false;
  }

  // Unsigned 8-bit arithmetic computes modulo 256 — this is not
  // appropriate when you have 12 solenoids, it causes pixel -1 to
  // have more than 1 solenoid of difference from pixel 0.
  // So instead we use a 16-bit int to compute the pixel, then
  // convert it back to an unsigned 8-bit integer using a multiple
  // of the number of solenoids as the modulus.
  // We only handle the underflow case, because the machine with 12
  // solenoids (KH270) has only 112 needles and can therefore never
  // have positions that cause an 8-bit overflow.
  int pixelToSet = (int)m_position - startOffset;

  if (pixelToSet < 0) {
    pixelToSet += Machine_t::Kh270 == m_machineType ? 252 : 256;
  }

  m_pixelToSet = pixelToSet;

  if (!beltShift) {
    m_solenoidToSet = (m_pixelToSet + bulkyOffset) % SOLENOIDS_NUM[static_cast<uint8_t>(m_machineType)];
  } else {
    m_solenoidToSet = (m_pixelToSet + HALF_SOLENOIDS_NUM[static_cast<uint8_t>(m_machineType)]) % SOLENOIDS_NUM[static_cast<uint8_t>(m_machineType)];
  }

  // The 270 has 12 solenoids but they get shifted over 3 bits
  if (m_machineType == Machine_t::Kh270) {
    m_solenoidToSet = m_solenoidToSet + 3;
  }
  return true;
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
