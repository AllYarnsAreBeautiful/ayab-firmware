/*!
 * \file global_test.cpp
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

#include "test.h"

// static member functions

void GlobalTest::init() {
  m_instance->init();
}

Err_t GlobalTest::begin() {
  return m_instance->begin();
}

void GlobalTest::update() {
  m_instance->update();
}

void GlobalTest::com() {
  m_instance->com();
}

void GlobalTest::end() {
  m_instance->end();
}


bool GlobalTest::enabled() {
  return m_instance->enabled();
}

void GlobalTest::helpCmd() {
  m_instance->helpCmd();
}

void GlobalTest::sendCmd() {
  m_instance->sendCmd();
}

void GlobalTest::beepCmd() {
  m_instance->beepCmd();
}

void GlobalTest::setSingleCmd(const uint8_t *buffer, size_t size) {
  m_instance->setSingleCmd(buffer, size);
}

void GlobalTest::setAllCmd(const uint8_t *buffer, size_t size) {
  m_instance->setAllCmd(buffer, size);
}

void GlobalTest::readEOLsensorsCmd() {
  m_instance->readEOLsensorsCmd();
}

void GlobalTest::readEncodersCmd() {
  m_instance->readEncodersCmd();
}

void GlobalTest::autoReadCmd() {
  m_instance->autoReadCmd();
}

void GlobalTest::autoTestCmd() {
  m_instance->autoTestCmd();
}

void GlobalTest::stopCmd() {
  m_instance->stopCmd();
}

#ifndef AYAB_TESTS
void GlobalTest::encoderAChange() {
  m_instance->encoderAChange();
}
#endif // AYAB_TESTS
