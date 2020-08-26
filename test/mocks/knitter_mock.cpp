/*!`
 * \file knitter_mock.cpp
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

#include <knitter.h>
#include <knitter_mock.h>

static KnitterMock *gKnitterMock = NULL;
KnitterMock *knitterMockInstance() {
  if (!gKnitterMock) {
    gKnitterMock = new KnitterMock();
  }
  return gKnitterMock;
}

void releaseKnitterMock() {
  if (gKnitterMock) {
    delete gKnitterMock;
    gKnitterMock = NULL;
  }
}

void Knitter::init() {
  assert(gKnitterMock != NULL);
  gKnitterMock->init();
}

void Knitter::setUpInterrupt() {
  assert(gKnitterMock != NULL);
  gKnitterMock->setUpInterrupt();
}

void Knitter::isr() {
  assert(gKnitterMock != NULL);
  gKnitterMock->isr();
}

Err_t Knitter::startKnitting(Machine_t machineType, uint8_t startNeedle,
                             uint8_t stopNeedle, uint8_t *pattern_start,
                             bool continuousReportingEnabled) {
  assert(gKnitterMock != NULL);
  return gKnitterMock->startKnitting(machineType, startNeedle, stopNeedle,
                                     pattern_start, continuousReportingEnabled);
}

void Knitter::encodePosition() {
  assert(gKnitterMock != NULL);
  gKnitterMock->encodePosition();
}

bool Knitter::isReady() {
  assert(gKnitterMock != NULL);
  return gKnitterMock->isReady();
}

void Knitter::knit() {
  assert(gKnitterMock != NULL);
  gKnitterMock->knit();
}

void Knitter::indState(Err_t error) {
  assert(gKnitterMock != NULL);
  gKnitterMock->indState(error);
}

uint8_t Knitter::getStartOffset(const Direction_t direction) {
  assert(gKnitterMock != NULL);
  return gKnitterMock->getStartOffset(direction);
}

Machine_t Knitter::getMachineType() {
  assert(gKnitterMock != NULL);
  return gKnitterMock->getMachineType();
}

bool Knitter::setNextLine(uint8_t lineNumber) {
  assert(gKnitterMock != NULL);
  return gKnitterMock->setNextLine(lineNumber);
}

void Knitter::setLastLine() {
  assert(gKnitterMock != NULL);
  gKnitterMock->setLastLine();
}

void Knitter::setMachineType(Machine_t machineType) {
  assert(gKnitterMock != NULL);
  return gKnitterMock->setMachineType(machineType);
}
