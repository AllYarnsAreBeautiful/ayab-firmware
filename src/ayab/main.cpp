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
 *    Modified Work Copyright 2020-3 Sturla Lange, Tom Price
 *    http://ayab-knitting.com
 */

#include <Arduino.h>

#include "beeper.h"
#include "com.h"
#include "encoders.h"
#include "fsm.h"
#include "solenoids.h"

#include "opIdle.h"
#include "opInit.h"
#include "opReady.h"
#include "opKnit.h"
#include "opTest.h"
#include "opError.h"

// Global definitions: references elsewhere must use `extern`.
// Each of the following is a pointer to a singleton class
// containing static methods.
constexpr GlobalBeeper    *beeper;
constexpr GlobalCom       *com;
constexpr GlobalEncoders  *encoders;
constexpr GlobalFsm       *fsm;
constexpr GlobalSolenoids *solenoids;

constexpr GlobalOpIdle    *opIdle;
constexpr GlobalOpInit    *opInit;
constexpr GlobalOpReady   *opReady;
constexpr GlobalOpKnit    *opKnit;
constexpr GlobalOpTest    *opTest;
constexpr GlobalOpError   *opError;

// Initialize static members.
// Each singleton class contains a pointer to a static instance
// that implements a public interface. When testing, a pointer
// to an instance of a mock class can be substituted.
BeeperInterface    *GlobalBeeper::m_instance    = new Beeper();
ComInterface       *GlobalCom::m_instance       = new Com();
EncodersInterface  *GlobalEncoders::m_instance  = new Encoders();
FsmInterface       *GlobalFsm::m_instance       = new Fsm();
SolenoidsInterface *GlobalSolenoids::m_instance = new Solenoids();

OpIdleInterface    *GlobalOpIdle::m_instance    = new OpIdle();
OpInitInterface    *GlobalOpInit::m_instance    = new OpInit();
OpReadyInterface   *GlobalOpReady::m_instance   = new OpReady();
OpKnitInterface    *GlobalOpKnit::m_instance    = new OpKnit();
OpTestInterface    *GlobalOpTest::m_instance    = new OpTest();
OpErrorInterface   *GlobalOpError::m_instance   = new OpError();

/*!
 * Setup - do once before going to the main loop.
 */
void setup() {
  // Objects running in async context
  GlobalBeeper::init(false);
  GlobalCom::init();
  GlobalFsm::init();
  GlobalSolenoids::init();

  GlobalOpKnit::init();
}

/*!
 * Main Loop - repeat forever.
 */
void loop() {
  // Non-blocking methods
  // Cooperative Round Robin scheduling
  GlobalFsm::update();
  GlobalCom::update();
  if (GlobalBeeper::enabled()) {
    GlobalBeeper::update();
  }
}
