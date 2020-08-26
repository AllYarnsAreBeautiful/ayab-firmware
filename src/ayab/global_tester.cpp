/*!
 * \file global_tester.cpp
 * \brief Singleton class containing methods for hardware testing.
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

#include "tester.h"

// static member functions

Err_t GlobalTester::startTest(Machine_t machineType) {
  return m_instance->startTest(machineType);
}

void GlobalTester::loop() {
  m_instance->loop();
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

void GlobalTester::setSingleCmd(const uint8_t *buffer, size_t size) {
  m_instance->setSingleCmd(buffer, size);
}

void GlobalTester::setAllCmd(const uint8_t *buffer, size_t size) {
  m_instance->setAllCmd(buffer, size);
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

#ifndef AYAB_TESTS
void GlobalTester::encoderAChange() {
  m_instance->encoderAChange();
}
#endif // AYAB_TESTS
