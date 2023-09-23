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
#include "encoders.h"
#include "fsm.h"

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
 * \brief OpInitialize state OpInit
 */
void OpInit::init() {
}

/*!
 * \brief Start state OpInit
 */
void OpInit::begin() {
  GlobalEncoders::init(GlobalFsm::getMachineType());
  GlobalEncoders::setUpInterrupt();
  digitalWrite(LED_PIN_A, LOW); // green LED off
}

/*!
 * \brief Update method for state OpInit
 */
void OpInit::update() {
  if (GlobalOpKnit::isReady()) {
    GlobalFsm::setState(GlobalOpReady::m_instance);
  }
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
