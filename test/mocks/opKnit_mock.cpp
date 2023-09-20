/*!`
 * \file op_knit_mock.cpp
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

#include <op_knit.h>
#include <op_knit_mock.h>

static OpKnitMock *gOpKnitMock = nullptr;
OpKnitMock *OpKnitMockInstance() {
  if (!gOpKnitMock) {
    gOpKnitMock = new OpKnitMock();
  }
  return gOpKnitMock;
}

void releaseOpKnitMock() {
  if (gOpKnitMock) {
    delete gOpKnitMock;
    gOpKnitMock = nullptr;
  }
}

void OpKnit::init() {
  assert(gOpKnitMock != nullptr);
  gOpKnitMock->init();
}

Err_t OpKnit::begin() {
  assert(gOpKnitMock != nullptr);
  return gOpKnitMock->begin();
}

void OpKnit::update() {
  assert(gOpKnitMock != nullptr);
  gOpKnitMock->update();
}

void OpKnit::com(const uint8_t *buffer, size_t size) {
  assert(gOpKnitMock != nullptr);
  gOpKnitMock->com(buffer, size);
}

void OpKnit::end() {
  assert(gOpKnitMock != nullptr);
  gOpKnitMock->end();
}

Err_t OpKnit::startOpKnitting(uint8_t startNeedle,
                             uint8_t stopNeedle, uint8_t *pattern_start,
                             bool continuousReportingEnabled) {
  assert(gOpKnitMock != nullptr);
  return gOpKnitMock->startOpKnitting(startNeedle, stopNeedle,
                                     pattern_start, continuousReportingEnabled);
}

void OpKnit::encodePosition() {
  assert(gOpKnitMock != nullptr);
  gOpKnitMock->encodePosition();
}

bool OpKnit::isReady() {
  assert(gOpKnitMock != nullptr);
  return gOpKnitMock->isReady();
}

void OpKnit::doOpKnitting() {
  assert(gOpKnitMock != nullptr);
  gOpKnitMock->doOpKnitting();
}

uint8_t OpKnit::getStartOffset(const Direction_t direction) {
  assert(gOpKnitMock != nullptr);
  return gOpKnitMock->getStartOffset(direction);
}

bool OpKnit::setNextLine(uint8_t lineNumber) {
  assert(gOpKnitMock != nullptr);
  return gOpKnitMock->setNextLine(lineNumber);
}

void OpKnit::setLastLine() {
  assert(gOpKnitMock != nullptr);
  gOpKnitMock->setLastLine();
}
