/*!`
 * \file op_test_mock.cpp
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

#include <op_test_mock.h>

static OpTestMock *gOpTestMock = nullptr;

OpTestMock *OpTestMockInstance() {
  if (!gOpTestMock) {
    gOpTestMock = new OpTestMock();
  }
  return gOpTestMock;
}

void releaseOpTestMock() {
  if (gOpTestMock) {
    delete gOpTestMock;
    gOpTestMock = nullptr;
  }
}

void OpTest::init() {
  assert(gOpTestMock != nullptr);
  gOpTestMock->init();
}

Err_t OpTest::begin() {
  assert(gOpTestMock != nullptr);
  return gOpTestMock->begin();
}

void OpTest::update() {
  assert(gOpTestMock != nullptr);
  gOpTestMock->update();
}

void OpTest::com(const uint8_t *buffer, size_t size) {
  assert(gOpTestMock != nullptr);
  gOpTestMock->com(buffer, size);
}

void OpTest::end() {
  assert(gOpTestMock != nullptr);
  gOpTestMock->end();
}

bool OpTest::enabled() {
  assert(gOpTestMock != nullptr);
  return gOpTestMock->enabled();
}

void OpTest::helpCmd() {
  assert(gOpTestMock != nullptr);
  gOpTestMock->helpCmd();
}

void OpTest::sendCmd() {
  assert(gOpTestMock != nullptr);
  gOpTestMock->sendCmd();
}

void OpTest::beepCmd() {
  assert(gOpTestMock != nullptr);
  gOpTestMock->beepCmd();
}

void OpTest::setSingleCmd(const uint8_t *buffer, size_t size) {
  assert(gOpTestMock != nullptr);
  gOpTestMock->setSingleCmd(buffer, size);
}

void OpTest::setAllCmd(const uint8_t *buffer, size_t size) {
  assert(gOpTestMock != nullptr);
  gOpTestMock->setAllCmd(buffer, size);
}

void OpTest::readEOLsensorsCmd() {
  assert(gOpTestMock != nullptr);
  gOpTestMock->readEOLsensorsCmd();
}

void OpTest::readEncodersCmd() {
  assert(gOpTestMock != nullptr);
  gOpTestMock->readEncodersCmd();
}

void OpTest::autoReadCmd() {
  assert(gOpTestMock != nullptr);
  gOpTestMock->autoReadCmd();
}

void OpTest::autoTestCmd() {
  assert(gOpTestMock != nullptr);
  gOpTestMock->autoTestCmd();
}

void OpTest::stopCmd() {
  assert(gOpTestMock != nullptr);
  gOpTestMock->stopCmd();
}
