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
 *    Modified Work Copyright 2020 Sturla Lange, Tom Price
 *    http://ayab-knitting.com
 */

#include "board.h"
#include <Arduino.h>

#include "com.h"
#include "fsm.h"
#include "knitter.h"

// Public methods

void Fsm::init() {
  m_opState = s_init;
}

OpState_t Fsm::getState() {
  return m_opState;
}

void Fsm::setState(OpState_t state) {
  m_opState = state;
}

/*!
 * \brief Dispatch on machine state
 *
 * \todo TP: add error state(s)
 */
void Fsm::dispatch() {
  switch (m_opState) {
  case s_init:
    state_init();
    break;

  case s_ready:
    state_ready();
    break;

  case s_knit:
    state_knit();
    break;

  case s_test:
    state_test();
    break;

  default:
    break;
  }
  GlobalCom::update();
}

// Private methods

void Fsm::state_init() {
  if (GlobalKnitter::isReady()) {
    setState(s_ready);
  }
}

void Fsm::state_ready() {
  digitalWrite(LED_PIN_A, 0); // green LED off
}

void Fsm::state_knit() {
  digitalWrite(LED_PIN_A, 1); // green LED on
  GlobalKnitter::knit();
}

void Fsm::state_test() {
  GlobalKnitter::encodePosition();
  GlobalTester::loop();
  if (GlobalTester::getQuitFlag()) {
    // return to state `s_init	 after quitting test
    GlobalKnitter::init();
    setState(s_init);
  }
}
