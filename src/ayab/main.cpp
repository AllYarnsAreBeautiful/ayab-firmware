/*!
 * \file main.cpp
 * \brief Main entry point of ayab-firmware.
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

#ifdef AYAB_HW_TEST
#include "hw_test.h"
#endif

#include "knitter.h"

/* Global Declarations */
Knitter *knitter; ///< A pointer to the global instance of the knitter object.

/*!
 * Setup - steps to take before going to the main loop.
 */
void setup() {
  knitter = new Knitter();
#ifdef AYAB_HW_TEST
  hw_test_setup();
#endif
}

/*!
 * Main Loop - repeated until the heat death of the universe,
 * or someone cuts power to us.
 */
void loop() {
#ifdef AYAB_HW_TEST
  hw_test_loop();
#else
  knitter->fsm();
#endif
}
