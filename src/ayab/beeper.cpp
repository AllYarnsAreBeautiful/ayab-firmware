/*!
 * \file beeper.cpp
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
 *    Copyright 2013 Christian Obersteiner, Andreas Müller
 *    http://ayab-knitting.com
 */

#include <Arduino.h>

#include "beeper.h"

Beeper::Beeper() {
  // Intentionally left blank
}

/*!
 * Beep to indicate readiness
 */
void Beeper::ready() {
  beep(5);
}

/*!
 * Beep to indicate the end of a line
 */
void Beeper::finishedLine() {
  beep(3);
}

/*!
 * Beep to indicate the end the knitting pattern
 */
void Beeper::endWork() {
  beep(10);
}

/* Private Methods */

/*!
 * Generic beep function.
 */
void Beeper::beep(uint8_t length) {
  for (int i = 0; i < length; i++) {
    analogWrite(PIEZO_PIN, 0);
    delay(BEEPDELAY);
    analogWrite(PIEZO_PIN, 20);
    delay(BEEPDELAY);
  }
  analogWrite(PIEZO_PIN, 255);
}
