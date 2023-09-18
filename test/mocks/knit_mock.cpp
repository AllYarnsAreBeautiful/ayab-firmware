/*!`
 * \file knit_mock.cpp
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

#include <knit.h>
#include <knit_mock.h>

static KnitMock *gKnitMock = nullptr;
KnitMock *knitMockInstance() {
  if (!gKnitMock) {
    gKnitMock = new KnitMock();
  }
  return gKnitMock;
}

void releaseKnitMock() {
  if (gKnitMock) {
    delete gKnitMock;
    gKnitMock = nullptr;
  }
}

void Knit::init() {
  assert(gKnitMock != nullptr);
  gKnitMock->init();
}

Err_t Knit::startKnitting(uint8_t startNeedle,
                             uint8_t stopNeedle, uint8_t *pattern_start,
                             bool continuousReportingEnabled) {
  assert(gKnitMock != nullptr);
  return gKnitMock->startKnitting(startNeedle, stopNeedle,
                                     pattern_start, continuousReportingEnabled);
}

Err_t Knit::initMachine(Machine_t machineType) {
  assert(gKnitMock != nullptr);
  return gKnitMock->initMachine(machineType);
}

void Knit::encodePosition() {
  assert(gKnitMock != nullptr);
  gKnitMock->encodePosition();
}

bool Knit::isReady() {
  assert(gKnitMock != nullptr);
  return gKnitMock->isReady();
}

void Knit::knit() {
  assert(gKnitMock != nullptr);
  gKnitMock->knit();
}

uint8_t Knit::getStartOffset(const Direction_t direction) {
  assert(gKnitMock != nullptr);
  return gKnitMock->getStartOffset(direction);
}

bool Knit::setNextLine(uint8_t lineNumber) {
  assert(gKnitMock != nullptr);
  return gKnitMock->setNextLine(lineNumber);
}

void Knit::setLastLine() {
  assert(gKnitMock != nullptr);
  gKnitMock->setLastLine();
}
