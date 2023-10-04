/*!
 * \file controller.cpp
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

#include "board.h"
#include <util/atomic.h>

#include "encoders.h"
#include "controller.h"

#include "opIdle.h"

// Public methods

/*!
 * \brief Return current machine state
 */
OpState_t Controller::state() {
  return m_currentState->state();
}

/*!
 * \brief Initialize Finite State Machine.
 */
void Controller::init() {
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

  m_machineType = Machine_t::NoMachine;
  m_carriage = Carriage_t::NoCarriage;
  m_direction = Direction_t::NoDirection;
  m_hallActive = Direction_t::NoDirection;
  m_beltShift = BeltShift_t::Unknown;
  m_position = 0;
  m_currentState = &GlobalOpIdle::m_instance;
  m_nextState = &GlobalOpIdle::m_instance;
}

/*!
 * \brief Call communication method of current machine state
 */
void Controller::com(const uint8_t *buffer, size_t size) {
  m_currentState->com(buffer, size);
}

/*!
 * \brief Dispatch on machine state; update machine state
 */
void Controller::update() {
  cacheEncoders();
  m_currentState->update();

  if (m_currentState == m_nextState) {
    return;
  }

  // else
  m_currentState->end();
  m_nextState->begin();
  m_currentState = m_nextState;
}

/*!
 * \brief Cache Encoder values
 * The code that saves the Encoder values is bookended by macros
 * ensuring that interrupts are disabled while the code executes.
 */
void Controller::cacheEncoders() {
#ifndef AYAB_TESTS
  ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
  // start of critical code
#endif
  m_beltShift  = GlobalEncoders::getBeltShift();
  m_carriage   = GlobalEncoders::getCarriage();
  m_direction  = GlobalEncoders::getDirection();
  m_hallActive = GlobalEncoders::getHallActive();
  m_position   = GlobalEncoders::getPosition();
#ifndef AYAB_TESTS
  // end of critical code
  }
#endif
}

/*!
 * \brief Set machine state.
 * \param state State.
 *
 * Does not take effect until next `update()`
 */
void Controller::setState(OpInterface *state) {
  m_nextState = state;
}

/*!
 * \brief Get machine state.
 * \return Current state of Finite State Machine.
 */
OpInterface* Controller::getState() {
  return m_currentState;
}

/*!
 * \brief Set machine type.
 * \param Machine type.
 */
void Controller::setMachineType(Machine_t machineType) {
  m_machineType = machineType;
}

/*!
 * \brief Get knitting machine type.
 * \return Machine type.
 */
Machine_t Controller::getMachineType() {
  return m_machineType;
}

/*!
 * \brief Get cached beltShift value.
 * \return Cached beltShift value.
 */
BeltShift_t Controller::getBeltShift() {
  return m_beltShift;
}

/*!
 * \brief Get cached carriage value.
 * \return Cached carriage value.
 */
Carriage_t Controller::getCarriage() {
  return m_carriage;
}

/*!
 * \brief Get cached direction value.
 * \return Cached direction value.
 */
Direction_t Controller::getDirection() {
  return m_direction;
}

/*!
 * \brief Get cached hallActive value.
 * \return Cached hallActive value.
 */
Direction_t Controller::getHallActive() {
  return m_hallActive;
}

/*!
 * \brief Get cached position value.
 * \return Cached position value.
 */
uint8_t Controller::getPosition() {
  return m_position;
}
