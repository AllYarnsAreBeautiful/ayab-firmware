/*!`
 * \file test_all.cpp
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

#include <analogReadAsyncWrapper.h>
#include <packetSerialWrapper.h>

#include <controller.h>
#include <opKnit.h>

#include <beeper_mock.h>
#include <com_mock.h>
#include <encoders_mock.h>
#include <solenoids_mock.h>

#include <opIdle_mock.h>
#include <opInit_mock.h>
#include <opReady_mock.h>
#include <opTest_mock.h>
#include <opError_mock.h>

// global definitions
// references everywhere else must use `extern`
BeeperMock&    beeper     = *new BeeperMock();
ComMock&       com        = *new ComMock();
EncodersMock&  encoders   = *new EncodersMock();
SolenoidsMock& solenoids  = *new SolenoidsMock();

OpIdleMock&    opIdle     = *new OpIdleMock();
OpInitMock&    opInit     = *new OpInitMock();
OpReadyMock&   opReady    = *new OpReadyMock();
OpTestMock&    opTest     = *new OpTestMock();
OpErrorMock&   opError    = *new OpErrorMock();

Controller&    controller = *new Controller();
OpKnit&        opKnit     = *new OpKnit();

AnalogReadAsyncWrapper& analogReadAsyncWrapper = *new AnalogReadAsyncWrapper();
PacketSerialWrapper&    packetSerialWrapper    = *new PacketSerialWrapper();

// instantiate singleton classes with mock objects
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
