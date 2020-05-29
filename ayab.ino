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
#ifdef AYAB_HW_TEST
#include "hw_test.h"
#else
#include "knitter.h"
#endif

/*
 * DEFINES
 */

/*
 *  DECLARATIONS
 */
#if !defined(AYAB_HW_TEST)
Knitter *knitter;

void isr_wrapper() {
  knitter->isr();
}
#endif
/*
 * SETUP
 */
void setup() {
#ifdef AYAB_HW_TEST
  hw_test_setup();
#else
  // Attaching ENC_PIN_A(=2), Interrupt No. 0
  attachInterrupt(0, isr_wrapper, CHANGE);

  knitter = new Knitter();
#endif
}

void loop() {
#ifdef AYAB_HW_TEST
  hw_test_loop();
#else
  knitter->fsm();
#endif
}
