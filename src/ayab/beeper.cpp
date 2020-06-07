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
#include "board.h"

/*!
 * Beep to indicate readiness
 */
void Beeper::ready() {
  beep(BEEP_NUM_READY);
}

/*!
 * Beep to indicate the end of a line
 */
void Beeper::finishedLine() {
  beep(BEEP_NUM_FINISHEDLINE);
}

/*!
 * Beep to indicate the end the knitting pattern
 */
void Beeper::endWork() {
  beep(BEEP_NUM_ENDWORK);
}

/* Private Methods */

/*!
 * Generic beep function.
 */
void Beeper::beep(uint8_t length) {

  for (uint8_t i = 0U; i < length; ++i) {

    analogWrite(PIEZO_PIN, BEEP_ON_DUTY);
    delay(BEEP_DELAY);

    analogWrite(PIEZO_PIN, BEEP_OFF_DUTY);
    delay(BEEP_DELAY);
  }

  analogWrite(PIEZO_PIN, BEEP_NO_DUTY);
}