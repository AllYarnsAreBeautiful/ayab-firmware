// ayab.ino
/*
This file is part of AYAB.

    AYAB is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    AYAB is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with AYAB.  If not, see <http://www.gnu.org/licenses/>.

    Copyright 2013 Christian Obersteiner, Andreas Müller
    http://ayab-knitting.com
*/

/*
 * INCLUDES
 */
#include "knitter.h"

/*
 * DEFINES
 */

/*
 *  DECLARATIONS
 */
Knitter *knitter;

void isr_wrapper() {
  knitter->isr();
}
/*
 * SETUP
 */
void setup() {
  // Attaching ENC_PIN_A(=2), Interrupt No. 0
  attachInterrupt(0, isr_wrapper, CHANGE);

  knitter = new Knitter();
}

void loop() {
  knitter->fsm();
}
