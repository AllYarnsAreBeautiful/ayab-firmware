/*!
 * \file global_hw_test.cpp
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

#include "global_hw_test.h"

// static member functions

void GlobalHardwareTest::setUp() {
  m_instance->setUp();
}

void GlobalHardwareTest::loop() {
  m_instance->loop();
}

#ifndef AYAB_TESTS
void GlobalHardwareTest::encoderAChange() {
  m_instance->encoderAChange();
}
#endif // AYAB_TESTS

/*
void GlobalHardwareTest::helpCmd() {
  m_instance->helpCmd();
}

void GlobalHardwareTest::sendCmd() {
  m_instance->sendCmd();
}

void GlobalHardwareTest::beepCmd() {
  m_instance->beepCmd();
}

void GlobalHardwareTest::setSingleCmd() {
  m_instance->setSingleCmd();
}

void GlobalHardwareTest::setAllCmd() {
  m_instance->setAllCmd();
}

void GlobalHardwareTest::readEOLsensorsCmd() {
  m_instance->readEOLsensorsCmd();
}

void GlobalHardwareTest::readEncodersCmd() {
  m_instance->readEncodersCmd();
}

void GlobalHardwareTest::autoReadCmd() {
  m_instance->autoReadCmd();
}

void GlobalHardwareTest::autoTestCmd() {
  m_instance->autoTestCmd();
}

void GlobalHardwareTest::stopCmd() {
  m_instance->stopCmd();
}

void GlobalHardwareTest::quitCmd() {
  m_instance->quitCmd();
}

void GlobalHardwareTest::unrecognizedCmd(const char *buffer) {
  m_instance->unrecognizedCmd(buffer);
}
*/
