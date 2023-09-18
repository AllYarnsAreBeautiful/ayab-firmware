/*!
 * \file global_OpTest.cpp
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

#include "opTest.h"

// static member functions

void GlobalOpTest::init() {
  m_instance->init();
}

Err_t GlobalOpTest::begin() {
  return m_instance->begin();
}

void GlobalOpTest::update() {
  m_instance->update();
}

void GlobalOpTest::com(const uint8_t *buffer, size_t size) {
  m_instance->com(buffer, size);
}

void GlobalOpTest::end() {
  m_instance->end();
}


bool GlobalOpTest::enabled() {
  return m_instance->enabled();
}

void GlobalOpTest::helpCmd() {
  m_instance->helpCmd();
}

void GlobalOpTest::sendCmd() {
  m_instance->sendCmd();
}

void GlobalOpTest::beepCmd() {
  m_instance->beepCmd();
}

void GlobalOpTest::setSingleCmd(const uint8_t *buffer, size_t size) {
  m_instance->setSingleCmd(buffer, size);
}

void GlobalOpTest::setAllCmd(const uint8_t *buffer, size_t size) {
  m_instance->setAllCmd(buffer, size);
}

void GlobalOpTest::readEOLsensorsCmd() {
  m_instance->readEOLsensorsCmd();
}

void GlobalOpTest::readEncodersCmd() {
  m_instance->readEncodersCmd();
}

void GlobalOpTest::autoReadCmd() {
  m_instance->autoReadCmd();
}

void GlobalOpTest::autoTestCmd() {
  m_instance->autoTestCmd();
}

void GlobalOpTest::stopCmd() {
  m_instance->stopCmd();
}

#ifndef AYAB_TESTS
void GlobalOpTest::encoderAChange() {
  m_instance->encoderAChange();
}
#endif // AYAB_TESTS
