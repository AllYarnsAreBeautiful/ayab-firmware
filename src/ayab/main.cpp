/*!
 * \file main.cpp
 * \brief Main entry point for AYAB firmware.
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
 *    Modified Work Copyright 2020 Sturla Lange, Tom Price
 *    http://ayab-knitting.com
 */

#include <Arduino.h>

#include "knitter.h"
#include "tester.h"

// global definitions
// references everywhere else must use `extern`
GlobalKnitter *knitter;
GlobalBeeper *beeper;
GlobalCom *com;
GlobalTester *tester;

// initialize static members
KnitterInterface *GlobalKnitter::m_instance = new Knitter();
BeeperInterface *GlobalBeeper::m_instance = new Beeper();
ComInterface *GlobalCom::m_instance = new Com();
TesterInterface *GlobalTester::m_instance = new Tester();

/*!
 * Setup - do once before going to the main loop.
 */
void setup() {
  GlobalKnitter::init();
  GlobalCom::init();
}

/*!
 * Main Loop - repeat forever.
 */
void loop() {
  GlobalKnitter::fsm();
}
