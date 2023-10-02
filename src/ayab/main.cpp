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

#include "analogReadAsyncWrapper.h"
#include "packetSerialWrapper.h"

#include "beeper.h"
#include "com.h"
#include "controller.h"
#include "encoders.h"
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
constexpr GlobalAnalogReadAsyncWrapper *analogReadasyncWrapper;
constexpr GlobalPacketSerialWrapper    *packetSerialWrapper;

constexpr GlobalBeeper     *beeper;
constexpr GlobalCom        *com;
constexpr GlobalController *controller;
constexpr GlobalEncoders   *encoders;
constexpr GlobalSolenoids  *solenoids;

constexpr GlobalOpIdle     *opIdle;
constexpr GlobalOpInit     *opInit;
constexpr GlobalOpReady    *opReady;
constexpr GlobalOpKnit     *opKnit;
constexpr GlobalOpTest     *opTest;
constexpr GlobalOpError    *opError;

// Initialize static members.
// Each singleton class contains a reference to a static instance
// that implements a public interface. When testing, a reference
// to an instance of a mock class can be substituted.
AnalogReadAsyncWrapperInterface& GlobalAnalogReadAsyncWrapper::m_instance = *new AnalogReadAsyncWrapper();
PacketSerialWrapperInterface&    GlobalPacketSerialWrapper::m_instance    = *new PacketSerialWrapper();

BeeperInterface&     GlobalBeeper::m_instance     = *new Beeper();
ComInterface&        GlobalCom::m_instance        = *new Com();
EncodersInterface&   GlobalEncoders::m_instance   = *new Encoders();
ControllerInterface& GlobalController::m_instance = *new Controller();
SolenoidsInterface&  GlobalSolenoids::m_instance  = *new Solenoids();

OpIdleInterface&     GlobalOpIdle::m_instance     = *new OpIdle();
OpInitInterface&     GlobalOpInit::m_instance     = *new OpInit();
OpReadyInterface&    GlobalOpReady::m_instance    = *new OpReady();
OpKnitInterface&     GlobalOpKnit::m_instance     = *new OpKnit();
OpTestInterface&     GlobalOpTest::m_instance     = *new OpTest();
OpErrorInterface&    GlobalOpError::m_instance    = *new OpError();

/*!
 * Setup - do once before going to the main loop.
 */
void setup() {
  // Objects running in async context
  GlobalBeeper::init(false);
  GlobalCom::init();
  GlobalController::init();
  GlobalSolenoids::init();
  GlobalOpKnit::init();
}

/*!
 * Main Loop - repeat forever.
 */
void loop() {
  // Non-blocking methods
  // Cooperative Round Robin scheduling
  GlobalController::update();
  GlobalCom::update();
  if (GlobalBeeper::enabled()) {
    GlobalBeeper::update();
  }
}
