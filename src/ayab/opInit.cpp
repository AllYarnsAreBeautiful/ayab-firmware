/*!
 * \file opInit.cpp
 * \brief Class containing methods for hardware testing.
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

#include <Arduino.h>
#include "board.h"

#include "com.h"
#include "controller.h"
#include "solenoids.h"

#include "opInit.h"
#include "opKnit.h"
#include "opReady.h"

/*!
 * \brief enum OpState
 * \return OpState_t::Init
 */
OpState_t OpInit::state() {
  return OpState_t::Init;
}

/*!
 * \brief Initialize state OpInit
 */
void OpInit::init() {
  m_lastHall = Direction_t::NoDirection;
#ifdef DBG_NOMACHINE
  m_prevState = false;
#endif
}

/*!
 * \brief Start state OpInit
 */
void OpInit::begin() {
  GlobalEncoders::init(GlobalController::getMachineType());
  GlobalEncoders::setUpInterrupt();
  digitalWrite(LED_PIN_A, LOW); // green LED off
}

/*!
 * \brief Update method for state OpInit
 * Assess whether the Finite State Machine is ready to move from state `OpInit` to `OpReady`.
 */
void OpInit::update() {
#ifdef DBG_NOMACHINE
  // TODO(who?): check if debounce is needed
  bool state = digitalRead(DBG_BTN_PIN);

  if (m_prevState && !state) {
#else
  // In order to support the garter carriage, we need to wait and see if there
  // will be a second magnet passing the sensor.
  // Keep track of the last seen Hall sensor because we may be making a decision
  // after it passes.
  auto hallActive = GlobalController::getHallActive();
  if (hallActive != Direction_t::NoDirection) {
    m_lastHall = hallActive;
  }

  auto direction = GlobalController::getDirection();
  auto position = GlobalController::getPosition();
  auto machineType = static_cast<uint8_t>(GlobalController::getMachineType());
  bool passedLeft = (Direction_t::Right == direction) && (Direction_t::Left == m_lastHall) &&
        (position > (END_LEFT_PLUS_OFFSET[machineType] + GARTER_SLOP));
  bool passedRight = (Direction_t::Left == direction) && (Direction_t::Right == m_lastHall) &&
        (position < (END_RIGHT_MINUS_OFFSET[machineType] - GARTER_SLOP));

  // Machine is initialized when Left Hall sensor is passed in Right direction
  // New feature (August 2020): the machine is also initialized
  // when the Right Hall sensor is passed in Left direction.
  if (passedLeft || passedRight) {

#endif // DBG_NOMACHINE
    GlobalSolenoids::setSolenoids(SOLENOIDS_BITMASK);
    GlobalCom::send_indState(Err_t::Success);
    // move to `OpReady`
    GlobalController::setState(GlobalOpReady::m_instance);
    return;
  }

#ifdef DBG_NOMACHINE
  m_prevState = state;
#endif
  // stay in `OpInit`
  return;
}

/*!
 * \brief Communication callback for state OpInit
 */
void OpInit::com(const uint8_t *buffer, size_t size) {
  (void) buffer;
  (void) size;
}

/*!
 * \brief Finish state OpInit
 */
void OpInit::end() {
}
