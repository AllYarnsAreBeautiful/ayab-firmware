/*!
 * \file opReady.cpp
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
#include "opReady.h"

/*!
 * \brief Initialize state OpReady
 */
void OpReady::init() {
}

/*!
 * \brief Start state OpReady
 * \return Error code (0 = success, other values = error).
 */
Err_t OpReady::begin() {
  digitalWrite(LED_PIN_A, LOW); // green LED off
  return ErrorCode::success;
}

/*!
 * \brief Update method for state OpReady
 */
void OpReady::update() {
}

/*!
 * \brief Communication callback for state OpReady
 */
void OpReady::com(const uint8_t *buffer, size_t size) {
  switch (buffer[0]) {
  case static_cast<uint8_t>(AYAB_API::reqStart):
    GlobalCom::h_reqStart(buffer, size);
    break;
  case static_cast<uint8_t>(AYAB_API::reqTest):
    GlobalCom::h_reqTest();
    break;
  default:
    break;
  }
}

/*!
 * \brief Finish state OpReady
 */
void OpReady::end() {
}
