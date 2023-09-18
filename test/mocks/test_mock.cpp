/*!`
 * \file test_mock.cpp
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

#include <test_mock.h>

static TestMock *gTestMock = nullptr;

TestMock *testMockInstance() {
  if (!gTestMock) {
    gTestMock = new TestMock();
  }
  return gTestMock;
}

void releaseTestMock() {
  if (gTestMock) {
    delete gTestMock;
    gTestMock = nullptr;
  }
}

Err_t Test::startTest(Machine_t machineType) {
  assert(gTestMock != nullptr);
  return gTestMock->startTest(machineType);
}

void Test::update() {
  assert(gTestMock != nullptr);
  gTestMock->update();

bool Test::enabled() {
  assert(gTestMock != nullptr);
  return gTestMock->enabled();
}

void Test::helpCmd() {
  assert(gTestMock != nullptr);
  gTestMock->helpCmd();
}

void Test::sendCmd() {
  assert(gTestMock != nullptr);
  gTestMock->sendCmd();
}

void Test::beepCmd() {
  assert(gTestMock != nullptr);
  gTestMock->beepCmd();
}

void Test::setSingleCmd(const uint8_t *buffer, size_t size) {
  assert(gTestMock != nullptr);
  gTestMock->setSingleCmd(buffer, size);
}

void Test::setAllCmd(const uint8_t *buffer, size_t size) {
  assert(gTestMock != nullptr);
  gTestMock->setAllCmd(buffer, size);
}

void Test::readEOLsensorsCmd() {
  assert(gTestMock != nullptr);
  gTestMock->readEOLsensorsCmd();
}

void Test::readEncodersCmd() {
  assert(gTestMock != nullptr);
  gTestMock->readEncodersCmd();
}

void Test::autoReadCmd() {
  assert(gTestMock != nullptr);
  gTestMock->autoReadCmd();
}

void Test::autoTestCmd() {
  assert(gTestMock != nullptr);
  gTestMock->autoTestCmd();
}

void Test::stopCmd() {
  assert(gTestMock != nullptr);
  gTestMock->stopCmd();
}

void Test::quitCmd() {
  assert(gTestMock != nullptr);
  gTestMock->quitCmd();
}
