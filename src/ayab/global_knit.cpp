/*!
 * \file global_knit.cpp
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
 *    Modified Work Copyright 2020 Sturla Lange, Tom Price
 *    http://ayab-knitting.com
 */

#include "knit.h"

// static member functions

void GlobalKnit::init() {
  m_instance->init();
}

Err_t GlobalKnit::begin() {
  return m_instance->begin();
}

void GlobalKnit::update() {
  m_instance->update();
}

void GlobalKnit::com() {
  m_instance->com();
}

void GlobalKnit::end() {
  m_instance->end();
}


Err_t GlobalKnit::startKnitting(uint8_t startNeedle,
                                   uint8_t stopNeedle, uint8_t *pattern_start,
                                   bool continuousReportingEnabled) {
  return m_instance->startKnitting(startNeedle, stopNeedle,
                                   pattern_start, continuousReportingEnabled);
}

void GlobalKnit::encodePosition() {
  m_instance->encodePosition();
}

bool GlobalKnit::isReady() {
  return m_instance->isReady();
}

void GlobalKnit::knit() {
  m_instance->knit();
}

uint8_t GlobalKnit::getStartOffset(const Direction_t direction) {
  return m_instance->getStartOffset(direction);
}

bool GlobalKnit::setNextLine(uint8_t lineNumber) {
  return m_instance->setNextLine(lineNumber);
}

void GlobalKnit::setLastLine() {
  m_instance->setLastLine();
}
