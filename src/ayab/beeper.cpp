/*!
 * \file beeper.cpp
 * \brief Class containing methods to actuate a beeper connected
 *    to PIEZO_PIN.
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
 *    Modified Work Copyright 2020-3 Sturla Lange, Tom Price
 *    http://ayab-knitting.com
 */

#include "beeper.h"
#include "board.h"

/*!
 * Initialize beeper
 */
void Beeper::init(bool enabled) {
  m_currentState = BeepState::idle;
  m_enabled = enabled;
}

/*!
 * Get beeper enabled flag
 */
bool Beeper::enabled() {
  return m_enabled;
}

/*!
 * Get beeper state
 */
BeepState Beeper::getState() {
  return m_currentState;
}

/*!
 * Beep to indicate readiness
 */
void Beeper::ready() {
  if (m_enabled) {
    beep(BEEP_NUM_READY);
  }
}

/*!
 * Beep to indicate the end of a line
 */
void Beeper::finishedLine() {
  if (m_enabled) {
    beep(BEEP_NUM_FINISHEDLINE);
  }
}

/*!
 * Beep to indicate the end the knitting pattern
 */
void Beeper::endWork() {
  if (m_enabled) {
    beep(BEEP_NUM_ENDWORK);
  }
}

/*!
 * Beep handler scheduled from main loop
 */
void Beeper::update() {
  uint32_t now = millis();
  switch (m_currentState) {
  case BeepState::on:
    analogWrite(PIEZO_PIN, BEEP_ON_DUTY);
    m_currentState = BeepState::wait;
    m_nextState = BeepState::off;
    m_nextTime = now + BEEP_DELAY;
    break;
  case BeepState::off:
    analogWrite(PIEZO_PIN, BEEP_OFF_DUTY);
    m_currentState = BeepState::wait;
    m_nextState = BeepState::on;
    m_nextTime = now + BEEP_DELAY;
    m_repeat--;
    break;
  case BeepState::wait:
    if (now >= m_nextTime) {
      if (m_repeat == 0) {
        analogWrite(PIEZO_PIN, BEEP_NO_DUTY);
        m_currentState = BeepState::idle;
      } else {
        m_currentState = m_nextState;
      }
    }
    break;
  case BeepState::idle:
  default:
    break;
  }
}

/* Private Methods */

/*!
 * Generic beep function.
 *
 * /param repeats number of beeps
 */
void Beeper::beep(uint8_t repeats) {
  m_repeat = repeats;
  m_currentState = BeepState::wait;
  m_nextState = BeepState::on;
  m_nextTime = millis();
}
