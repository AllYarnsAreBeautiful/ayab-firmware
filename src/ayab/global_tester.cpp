/*!
 * \file global_tester.cpp
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

#include "tester.h"

// static member functions

void GlobalTester::setUp() {
  m_instance->setUp();
}

void GlobalTester::loop() {
  m_instance->loop();
}

bool GlobalTester::getQuitFlag() {
  return m_instance->getQuitFlag();
}

void GlobalTester::helpCmd() {
  m_instance->helpCmd();
}

void GlobalTester::sendCmd() {
  m_instance->sendCmd();
}

void GlobalTester::beepCmd() {
  m_instance->beepCmd();
}

void GlobalTester::setSingleCmd() {
  m_instance->setSingleCmd();
}

void GlobalTester::setAllCmd() {
  m_instance->setAllCmd();
}

void GlobalTester::readEOLsensorsCmd() {
  m_instance->readEOLsensorsCmd();
}

void GlobalTester::readEncodersCmd() {
  m_instance->readEncodersCmd();
}

void GlobalTester::autoReadCmd() {
  m_instance->autoReadCmd();
}

void GlobalTester::autoTestCmd() {
  m_instance->autoTestCmd();
}

void GlobalTester::stopCmd() {
  m_instance->stopCmd();
}

void GlobalTester::quitCmd() {
  m_instance->quitCmd();
}

void GlobalTester::unrecognizedCmd(const char *buffer) {
  m_instance->unrecognizedCmd(buffer);
}

#ifndef AYAB_TESTS
void GlobalTester::encoderAChange() {
  m_instance->encoderAChange();
}
#endif // AYAB_TESTS
