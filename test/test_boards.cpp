/*!`
 * \file test_boards.cpp
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

#include "gtest/gtest.h"

#include <beeper.h>
#include <com.h>
#include <encoders.h>
#include <solenoids.h>

#include <opIdle.h>
#include <opInit.h>
#include <opReady.h>
#include <opTest.h>
#include <opError.h>

#include <analogReadAsyncWrapper_mock.h>
#include <packetSerialWrapper_mock.h>
#include <controller_mock.h>
#include <opKnit_mock.h>

// Global definitions
// References everywhere else must use `extern`
Beeper&    beeper    = *new Beeper();
Com&       com       = *new Com();
Encoders&  encoders  = *new Encoders();
Solenoids& solenoids = *new Solenoids();

OpIdle&    opIdle    = *new OpIdle();
OpInit&    opInit    = *new OpInit();
OpReady&   opReady   = *new OpReady();
OpTest&    opTest    = *new OpTest();
OpError&   opError   = *new OpError();

ControllerMock& controller = *new ControllerMock();
OpKnitMock&     opKnit     = *new OpKnitMock();

AnalogReadAsyncWrapperMock& analogReadAsyncWrapper = *new AnalogReadAsyncWrapperMock();
PacketSerialWrapperMock&    packetSerialWrapper    = *new PacketSerialWrapperMock();

// Initialize static members
BeeperInterface&     GlobalBeeper::m_instance     = beeper;
ComInterface&        GlobalCom::m_instance        = com;
EncodersInterface&   GlobalEncoders::m_instance   = encoders;
SolenoidsInterface&  GlobalSolenoids::m_instance  = solenoids;

OpIdleInterface&     GlobalOpIdle::m_instance     = opIdle;
OpInitInterface&     GlobalOpInit::m_instance     = opInit;
OpReadyInterface&    GlobalOpReady::m_instance    = opReady;
OpTestInterface&     GlobalOpTest::m_instance     = opTest;
OpErrorInterface&    GlobalOpError::m_instance    = opError;

ControllerInterface& GlobalController::m_instance = controller;
OpKnitInterface&     GlobalOpKnit::m_instance     = opKnit;

AnalogReadAsyncWrapperInterface& GlobalAnalogReadAsyncWrapper::m_instance = analogReadAsyncWrapper;
PacketSerialWrapperInterface&    GlobalPacketSerialWrapper::m_instance    = packetSerialWrapper;

int main(int argc, char *argv[]) {
  ::testing::InitGoogleMock(&argc, argv);
  return RUN_ALL_TESTS();
}
