/*!
 * \file global_OpKnit.cpp
 * \brief Singleton class containing methods for the finite state machine
 *    that co-ordinates the AYAB firmware.
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

#include "opKnit.h"

// static member functions

OpState_t GlobalOpKnit::state() {
  return m_instance->state();
}

void GlobalOpKnit::init() {
  m_instance->init();
}

void GlobalOpKnit::begin() {
  m_instance->begin();
}

void GlobalOpKnit::update() {
  m_instance->update();
}

void GlobalOpKnit::com(const uint8_t *buffer, size_t size) {
  m_instance->com(buffer, size);
}

void GlobalOpKnit::end() {
  m_instance->end();
}


Err_t GlobalOpKnit::startKnitting(uint8_t startNeedle,
                                   uint8_t stopNeedle, uint8_t *pattern_start,
                                   bool continuousReportingEnabled) {
  return m_instance->startKnitting(startNeedle, stopNeedle,
                                   pattern_start, continuousReportingEnabled);
}

void GlobalOpKnit::knit() {
  m_instance->knit();
}

uint8_t GlobalOpKnit::getStartOffset(const Direction_t direction) {
  return m_instance->getStartOffset(direction);
}

bool GlobalOpKnit::setNextLine(uint8_t lineNumber) {
  return m_instance->setNextLine(lineNumber);
}

void GlobalOpKnit::setLastLine() {
  m_instance->setLastLine();
}
