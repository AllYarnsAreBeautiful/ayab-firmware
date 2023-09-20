/*!
 * \file opError.cpp
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

#include "com.h"

#include "opError.h"
#include "opKnit.h"

/*!
 * \brief Initialize state OpError
 */
void OpError::init() {
}

/*!
 * \brief Start state OpError
 * \return OpError code (0 = success, other values = error).
 */
Err_t OpError::begin() {
  m_flash = false;
  m_flashTime = millis();
  m_error = ErrorCode::success;
  return ErrorCode::success;
}

/*!
 * \brief Update method for state OpError
 */
void OpError::update() {
  // every 500ms
  // send `indState` and flash LEDs
  uint32_t now = millis();
  if (now - m_flashTime >= FLASH_DELAY) {
    digitalWrite(LED_PIN_A, m_flash);  // green LED
    digitalWrite(LED_PIN_B, !m_flash); // yellow LED
    m_flash = !m_flash;
    m_flashTime = now;
    // send error message
    GlobalCom::send_indState(m_error);
  }
}

/*!
 * \brief Communication callback for state OpError
 */
void OpError::com(const uint8_t *buffer, size_t size) {
  // to avoid warning about unused parameters
  (void) buffer;
  (void) size;
}

/*!
 * \brief Finish state OpError
 */
void OpError::end() {
  digitalWrite(LED_PIN_A, LOW); // green LED off
  digitalWrite(LED_PIN_B, LOW); // yellow LED off
  GlobalOpKnit::init();
}
