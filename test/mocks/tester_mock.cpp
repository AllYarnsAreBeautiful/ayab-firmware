/*!`
 * \file tester_mock.cpp
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

#include <tester_mock.h>

static TesterMock *gTesterMock = nullptr;

TesterMock *testerMockInstance() {
  if (!gTesterMock) {
    gTesterMock = new TesterMock();
  }
  return gTesterMock;
}

void releaseTesterMock() {
  if (gTesterMock) {
    delete gTesterMock;
    gTesterMock = nullptr;
  }
}

Err_t Tester::startTest(Machine_t machineType) {
  assert(gTesterMock != nullptr);
  return gTesterMock->startTest(machineType);
}

void Tester::loop() {
  assert(gTesterMock != nullptr);
  gTesterMock->loop();
}

void Tester::helpCmd() {
  assert(gTesterMock != nullptr);
  gTesterMock->helpCmd();
}

void Tester::sendCmd() {
  assert(gTesterMock != nullptr);
  gTesterMock->sendCmd();
}

void Tester::beepCmd() {
  assert(gTesterMock != nullptr);
  gTesterMock->beepCmd();
}

void Tester::setSingleCmd(const uint8_t *buffer, size_t size) {
  assert(gTesterMock != nullptr);
  gTesterMock->setSingleCmd(buffer, size);
}

void Tester::setAllCmd(const uint8_t *buffer, size_t size) {
  assert(gTesterMock != nullptr);
  gTesterMock->setAllCmd(buffer, size);
}

void Tester::readEOLsensorsCmd() {
  assert(gTesterMock != nullptr);
  gTesterMock->readEOLsensorsCmd();
}

void Tester::readEncodersCmd() {
  assert(gTesterMock != nullptr);
  gTesterMock->readEncodersCmd();
}

void Tester::autoReadCmd() {
  assert(gTesterMock != nullptr);
  gTesterMock->autoReadCmd();
}

void Tester::autoTestCmd() {
  assert(gTesterMock != nullptr);
  gTesterMock->autoTestCmd();
}

void Tester::stopCmd() {
  assert(gTesterMock != nullptr);
  gTesterMock->stopCmd();
}

void Tester::quitCmd() {
  assert(gTesterMock != nullptr);
  gTesterMock->quitCmd();
}

void Tester::encoderChange() {
  assert(gTesterMock != nullptr);
  gTesterMock->encoderChange();
}
