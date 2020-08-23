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
 *    Modified Work Copyright 2020 Sturla Lange, Tom Price
 *    http://ayab-knitting.com
 */

#include "gtest/gtest.h"

#include <beeper.h>
#include <com.h>
#include <encoders.h>
#include <solenoids.h>
#include <tester.h>

#include <fsm_mock.h>
#include <knitter_mock.h>

// global definitions
// references everywhere else must use `extern`
Beeper *beeper = new Beeper();
Com *com = new Com();
Encoders *encoders = new Encoders();
Solenoids *solenoids = new Solenoids();
Tester *tester = new Tester();

FsmMock *fsm = new FsmMock();
KnitterMock *knitter = new KnitterMock();

// initialize static members
BeeperInterface *GlobalBeeper::m_instance = beeper;
ComInterface *GlobalCom::m_instance = com;
EncodersInterface *GlobalEncoders::m_instance = encoders;
SolenoidsInterface *GlobalSolenoids::m_instance = solenoids;
TesterInterface *GlobalTester::m_instance = tester;

FsmInterface *GlobalFsm::m_instance = fsm;
KnitterInterface *GlobalKnitter::m_instance = knitter;

int main(int argc, char *argv[]) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
