/*!
 * \file op.cpp
 * \brief Class containing methods for knit and test operations.
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

// GCOVR_EXCL_START
// There are some odd gaps in the `gcovr` coverage for this file.
// Maybe this could happen if there were missing `Mock::VerifyAndClear`
// statements in `test_fsm.cpp`.

#include "board.h"
#include <Arduino.h>
#include <util/atomic.h>

#include "com.h"
#include "knitter.h"
#include "op.h"

// Public methods

/*!
 * \brief Initialize Finite State Machine.
 */
void Op::init() {
  m_currentState = OpState::wait_for_machine;
  m_nextState = OpState::wait_for_machine;
  m_flash = false;
  m_flashTime = millis();
  m_error = ErrorCode::SUCCESS;
}

/*!
 * \brief Dispatch on machine state
 */
void Op::update() {
  cacheEncoders();
  switch (m_currentState) {
  case OpState::wait_for_machine:
    state_wait_for_machine();
    break;

  case OpState::init:
    state_init();
    break;

  case OpState::ready:
    state_ready();
    break;

  case OpState::knit:
    state_knit();
    break;

  case OpState::test:
    state_test();
    break;

  case OpState::error:
    state_error();
    break;

  default:
    break;
  }
  m_currentState = m_nextState;
}
// GCOVR_EXCL_STOP

/*!
 * \brief Cache Encoder values
 */
void Op::cacheEncoders() {
  // update machine state data
  ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
    m_beltShift  = GlobalEncoders::getBeltShift();
    m_carriage   = GlobalEncoders::getCarriage();
    m_direction  = GlobalEncoders::getDirection();
    m_hallActive = GlobalEncoders::getHallActive();
    m_position   = GlobalEncoders::getPosition();
  }
}

/*!
 * \brief Set machine state.
 * \param state State.
 *
 * Does not take effect until next `update()`
 */
void Op::setState(OpState_t state) {
  m_nextState = state;
}

/*!
 * \brief Get machine state.
 * \return Current state of Finite State Machine.
 */
OpState_t Op::getState() {
  return m_currentState;
}

/*!
 * \brief Get cached beltShift value.
 * \return Cached beltShift value.
 */
BeltShift_t Op::getBeltShift() {
  return m_beltShift;
}

/*!
 * \brief Get cached carriage value.
 * \return Cached carriage value.
 */
Carriage_t Op::getCarriage() {
  return m_carriage;
}

/*!
 * \brief Get cached direction value.
 * \return Cached direction value.
 */
Direction_t Op::getDirection() {
  return m_direction;
}

/*!
 * \brief Get cached hallActive value.
 * \return Cached hallActive value.
 */
Direction_t Op::getHallActive() {
  return m_hallActive;
}

/*!
 * \brief Get cached position value.
 * \return Cached position value.
 */
uint8_t Op::getPosition() {
  return m_position;
}


// Private methods

/*!
 * \brief Action of machine in state `wait_for_machine`.
 */
void Op::state_wait_for_machine() const {
  digitalWrite(LED_PIN_A, LOW); // green LED off
}

/*!
 * \brief Action of machine in state `OpState::init`.
 */
void Op::state_init() {
  digitalWrite(LED_PIN_A, LOW); // green LED off
  if (GlobalKnitter::isReady()) {
    setState(OpState::ready);
  }
}

/*!
 * \brief Action of machine in state `OpState::ready`.
 */
void Op::state_ready() const {
  digitalWrite(LED_PIN_A, LOW); // green LED off
}

/*!
 * \brief Action of machine in state `OpState::knit`.
 */
void Op::state_knit() const {
  digitalWrite(LED_PIN_A, HIGH); // green LED on
  GlobalKnitter::knit();
}

/*!
 * \brief Action of machine in state `OpState::test`.
 */
void Op::state_test() const {
}

/*!
 * \brief Action of machine in state `OpState::error`.
 */
void Op::state_error() {
  if (m_nextState == OpState::init) {
    // exit error state
    digitalWrite(LED_PIN_A, LOW); // green LED off
    digitalWrite(LED_PIN_B, LOW); // yellow LED off
    GlobalKnitter::init();
    return;
  }
  // every 500ms
  // send `indState` and flash LEDs
  unsigned long now = millis();
  if (now - m_flashTime >= FLASH_DELAY) {
    digitalWrite(LED_PIN_A, m_flash);  // green LED
    digitalWrite(LED_PIN_B, !m_flash); // yellow LED
    m_flash = !m_flash;
    m_flashTime = now;
    // send error message
    GlobalCom::send_indState(m_error);
  }
}
