/*!
 * \file ready.cpp
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

#include "com.h"
#include "ready.h"

/*!
 * \brief Initialize OpState::Ready
 */
void Ready::init() {
}

/*!
 * \brief Start OpState::Ready
 * \return Error code (0 = success, other values = error).
 */
Err_t Ready::begin() {
  return ErrorCode::success;
}

/*!
 * \brief Update method for OpState::Ready
 */
void Ready::update() {
  digitalWrite(LED_PIN_A, LOW); // green LED off
}

/*!
 * \brief Communication callback for OpState::Ready
 */
void Ready::com() {
}

/*!
 * \brief Finish OpState::Ready
 */
void Ready::end() {
}
