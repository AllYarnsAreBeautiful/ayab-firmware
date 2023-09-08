/*!
 * \file fsm.cpp
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
 *    Modified Work Copyright 2020-3 Sturla Lange, Tom Price
 *    http://ayab-knitting.com
 */

// GCOVR_EXCL_START
// There are some odd gaps in the `gcovr` coverage for this file.
// Maybe this could happen if there were missing `Mock::VerifyAndClear`
// statements in `test_fsm.cpp`.

#include "board.h"
#include <Arduino.h>

#include "com.h"
#include "fsm.h"
#include "knitter.h"

// Public methods

/*!
 * \brief Initialize Finite State Machine.
 */
void Fsm::init() {
  m_currentState = OpState::wait_for_machine;
  m_nextState = OpState::wait_for_machine;
  m_flash = false;
  m_flashTime = millis();
  m_error = SUCCESS;
}

/*!
 * \brief Get machine state.
 * \return Current state of Finite State Machine.
 */
OpState_t Fsm::getState() {
  return m_currentState;
}

/*!
 * \brief Set machine state.
 * \param state State.
 *
 * Does not take effect until next `dispatch()`
 */
void Fsm::setState(OpState_t state) {
  m_nextState = state;
}

/*!
 * \brief Dispatch on machine state
 */
void Fsm::dispatch() {
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
  GlobalCom::update();
}
// GCOVR_EXCL_STOP

// Private methods

/*!
 * \brief Action of machine in state `wait_for_machine`.
 */
void Fsm::state_wait_for_machine() {
  digitalWrite(LED_PIN_A, LOW); // green LED off
}

/*!
 * \brief Action of machine in state `OpState::init`.
 */
void Fsm::state_init() {
  digitalWrite(LED_PIN_A, LOW); // green LED off
  if (GlobalKnitter::isReady()) {
    setState(OpState::ready);
  }
}

/*!
 * \brief Action of machine in state `OpState::ready`.
 */
void Fsm::state_ready() {
  digitalWrite(LED_PIN_A, LOW); // green LED off
}

/*!
 * \brief Action of machine in state `OpState::knit`.
 */
const void Fsm::state_knit() {
  digitalWrite(LED_PIN_A, HIGH); // green LED on
  GlobalKnitter::knit();
}

/*!
 * \brief Action of machine in state `OpState::test`.
 */
const void Fsm::state_test() {
  GlobalKnitter::encodePosition();
  GlobalTester::loop();
  if (m_nextState == OpState::init) {
    // quit test
    GlobalKnitter::init();
  }
}

/*!
 * \brief Action of machine in state `OpState::error`.
 */
void Fsm::state_error() {
  if (m_nextState == OpState::init) {
    // exit error state
    digitalWrite(LED_PIN_B, LOW); // yellow LED off
    GlobalKnitter::init();
    return;
  }
  // every 500ms
  // send `indState` and flash LEDs
  unsigned long now = millis();
  if (now - m_flashTime >= 500) {
    digitalWrite(LED_PIN_A, m_flash);     // green LED
    digitalWrite(LED_PIN_B, not m_flash); // yellow LED
    m_flash = not m_flash;
    m_flashTime = now;

    // send error message
    GlobalKnitter::indState(m_error);
  }
}
