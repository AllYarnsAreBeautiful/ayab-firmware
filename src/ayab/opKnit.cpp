/*!
 * \file opKnit.cpp
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

#include <Arduino.h>
#include "board.h"

#include "beeper.h"
#include "com.h"
#include "controller.h"
#include "encoders.h"
#include "solenoids.h"

#include "opInit.h"
#include "opReady.h"
#include "opKnit.h"

#ifdef CLANG_TIDY
// clang-tidy doesn't find these macros for some reason,
// no problem when building or testing though.
constexpr uint8_t UINT8_MAX = 0xFFU;
constexpr uint16_t UINT16_MAX = 0xFFFFU;
#endif

/*!
 * \brief enum OpState
 * \return OpState_t::Knit
 */
OpState_t OpKnit::state() {
  return OpState_t::Knit;
}

/*!
 * \brief Initialize OpKnit object.
 *
 * Initialize the solenoids as well as pins and interrupts.
 */
void OpKnit::init() {
  // explicitly initialize members

  // job parameters
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

  m_solenoidToSet = 0U;
  m_pixelToSet = 0U;
}

/*!
 * \brief Start `OpKnit` operation.
 */
void OpKnit::begin() {
  GlobalEncoders::init(GlobalController::getMachineType());
  GlobalEncoders::setUpInterrupt();
}

/*!
 * \brief Update knitting operation.
 */
void OpKnit::update() {
  knit();
}

/*!
 * \brief Communication callback for knitting operation.
 */
void OpKnit::com(const uint8_t *buffer, size_t size) {
  switch (buffer[0]) {
  case static_cast<uint8_t>(API_t::cnfLine):
    GlobalCom::h_cnfLine(buffer, size);
    break;

  case static_cast<uint8_t>(API_t::reqTest):
    GlobalCom::h_reqTest();
    break;

  // FIXME needs to be a `Cancel` command in the API that resets state from `OpKnit` to `OpInit`
  default:
    GlobalCom::h_unrecognized();
    break;
  }
}

/*!
 * \brief Finish knitting operation.
 */
void OpKnit::end() {
  GlobalBeeper::endWork();
  GlobalSolenoids::setSolenoids(SOLENOIDS_BITMASK);

  // detaching ENC_PIN_A, Interrupt #0
  /* detachInterrupt(digitalPinToInterrupt(ENC_PIN_A)); */
}

/*!
 * \brief Obtain parameters of knitting pattern.
 * \param startNeedle Position of first needle in the pattern.
 * \param stopNeedle Position of last needle in the pattern.
 * \param patternStart Pointer to buffer containing pattern data.
 * \param continuousReportingEnabled Flag variable indicating whether the device continuously reports its status to the host.
 * \return Error code (0 = success, other values = error).
 */
Err_t OpKnit::startKnitting(uint8_t startNeedle,
                             uint8_t stopNeedle, uint8_t *pattern_start,
                             bool continuousReportingEnabled) {
  Machine_t machineType = GlobalController::getMachineType();
  if (machineType == Machine_t::NoMachine) {
    return Err_t::No_machine_type;
  }
  if (pattern_start == nullptr) {
    return Err_t::Null_pointer_argument;
  }
  if ((startNeedle >= stopNeedle) || (stopNeedle >= NUM_NEEDLES[static_cast<uint8_t>(machineType)])) {
    return Err_t::Needle_value_invalid;
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
  GlobalController::setState(GlobalOpKnit::m_instance);
  GlobalBeeper::ready();

  // success
  return Err_t::Success;
}

/*!
 * \brief Record current encoder position.
 *
 * Used in hardware test procedure.
 */
void OpKnit::encodePosition() {
  auto position = GlobalController::getPosition();
  if (m_sOldPosition != position) {
    // only act if there is an actual change of position
    // store current encoder position for next call of this function
    m_sOldPosition = position;
    (void) calculatePixelAndSolenoid();
    GlobalCom::send_indState(Err_t::Unspecified_failure); // FIXME is this the right error code?
  }
}

/*!
 * \brief Function that is repeatedly called during state `OpKnit`
 */
void OpKnit::knit() {
  if (m_firstRun) {
    m_firstRun = false;
    // TODO(who?): optimize delay for various Arduino models
    delay(START_KNITTING_DELAY);
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
  auto position = GlobalController::getPosition();
  // only act if there is an actual change of position
  if (m_sOldPosition == position) {
    return;
  }

  // store current carriage position for next call of this function
  m_sOldPosition = position;

  if (m_continuousReportingEnabled) {
    // send current position to GUI
    GlobalCom::send_indState(Err_t::Success);
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

  auto machineType = static_cast<uint8_t>(GlobalController::getMachineType());
  if (((m_pixelToSet < m_startNeedle - END_OF_LINE_OFFSET_L[machineType]) ||
       (m_pixelToSet > m_stopNeedle + END_OF_LINE_OFFSET_R[machineType])) &&
      m_workedOnLine) {
    // outside of the active needles and
    // already worked on the current line -> finished the line
    m_workedOnLine = false;

    if (!m_lineRequested && !m_lastLineFlag) {
      // request new line from host
      ++m_currentLineNumber;
      reqLine(m_currentLineNumber);
    } else if (m_lastLineFlag) {
      // move to state `OpInit`
      GlobalController::setState(GlobalOpInit::m_instance);
    }
  }
#endif // DBG_NOMACHINE
}

/*!
 * \brief Get start offset.
 * \return Start offset, or 0 if unobtainable.
 */
uint8_t OpKnit::getStartOffset(const Direction_t direction) {
  auto carriage = GlobalController::getCarriage();
  auto machineType = GlobalController::getMachineType();
  if ((direction == Direction_t::NoDirection) ||
      (carriage == Carriage_t::NoCarriage) ||
      (machineType == Machine_t::NoMachine)) {
    return 0U;
  }
  return START_OFFSET[static_cast<uint8_t>(machineType)][static_cast<uint8_t>(direction)][static_cast<uint8_t>(carriage)];
}

/*!
 * \brief Set line number of next row to be knitted.
 * \param lineNumber Line number (0-indexed and modulo 256).
 * \return `true` if successful, `false` otherwise.
 */
bool OpKnit::setNextLine(uint8_t lineNumber) {
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
void OpKnit::setLastLine() {
  m_lastLineFlag = true;
}

// private methods

/*!
 * \brief Send `reqLine` message.
 * \param lineNumber Line number requested.
 */
void OpKnit::reqLine(uint8_t lineNumber) {
  GlobalCom::send_reqLine(lineNumber, Err_t::Success);
  m_lineRequested = true;
}

/*!
 * \brief Calculate the solenoid and pixel to be set.
 * \return `true` if successful, `false` otherwise.
 */
bool OpKnit::calculatePixelAndSolenoid() {
  uint8_t startOffset = 0;

  auto position = GlobalController::getPosition();
  auto direction = GlobalController::getDirection();
  auto beltShift = GlobalController::getBeltShift();
  auto carriage = GlobalController::getCarriage();
  auto machineType = GlobalController::getMachineType();

  switch (direction) {
  // calculate the solenoid and pixel to be set
  // implemented according to machine manual
  // magic numbers from machine manual
  case Direction_t::Right:
    startOffset = getStartOffset(Direction_t::Left);
    if (position >= startOffset) {
      m_pixelToSet = position - startOffset;

      if ((BeltShift::Regular == beltShift) || (machineType == Machine_t::Kh270)) {
        m_solenoidToSet = position % SOLENOIDS_NUM[static_cast<uint8_t>(machineType)];
      } else if (BeltShift::Shifted == beltShift) {
        m_solenoidToSet = (position - HALF_SOLENOIDS_NUM[static_cast<uint8_t>(machineType)]) % SOLENOIDS_NUM[static_cast<uint8_t>(machineType)];
      }
      if (Carriage_t::Lace == carriage) {
        m_pixelToSet = m_pixelToSet + HALF_SOLENOIDS_NUM[static_cast<uint8_t>(machineType)];
      }
    } else {
      return false;
    }
    break;

  case Direction_t::Left:
    startOffset = getStartOffset(Direction_t::Right);
    if (position <= (END_RIGHT[static_cast<uint8_t>(machineType)] - startOffset)) {
      m_pixelToSet = position - startOffset;

      if ((BeltShift::Regular == beltShift) || (machineType == Machine_t::Kh270)) {
        m_solenoidToSet = (position + HALF_SOLENOIDS_NUM[static_cast<uint8_t>(machineType)]) % SOLENOIDS_NUM[static_cast<uint8_t>(machineType)];
      } else if (BeltShift::Shifted == beltShift) {
        m_solenoidToSet = position % SOLENOIDS_NUM[static_cast<uint8_t>(machineType)];
      }
      if (Carriage_t::Lace == carriage) {
        m_pixelToSet = m_pixelToSet - SOLENOIDS_NUM[static_cast<uint8_t>(machineType)];
      }
    } else {
      return false;
    }
    break;

  case Direction_t::NoDirection:
  default:
    return false;
  }
  // The 270 has 12 solenoids but they get shifted over 3 bits
  if (machineType == Machine_t::Kh270) {
    m_solenoidToSet = m_solenoidToSet + 3;
  }
  return true;
}
