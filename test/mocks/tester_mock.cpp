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
 *    Modified Work Copyright 2020 Sturla Lange, Tom Price
 *    http://ayab-knitting.com
 */

#include <tester_mock.h>

static TesterMock *gTesterMock = NULL;

TesterMock *testerMockInstance() {
  if (!gTesterMock) {
    gTesterMock = new TesterMock();
  }
  return gTesterMock;
}

void releaseTesterMock() {
  if (gTesterMock) {
    delete gTesterMock;
    gTesterMock = NULL;
  }
}

void Tester::setUp() {
  assert(gTesterMock != NULL);
  gTesterMock->setUp();
}

void Tester::loop() {
  assert(gTesterMock != NULL);
  gTesterMock->loop();
}

bool Tester::getQuitFlag() {
  assert(gTesterMock != NULL);
  return gTesterMock->getQuitFlag();
}

void Tester::helpCmd() {
  assert(gTesterMock != NULL);
  gTesterMock->helpCmd();
}

void Tester::sendCmd() {
  assert(gTesterMock != NULL);
  gTesterMock->sendCmd();
}

void Tester::beepCmd() {
  assert(gTesterMock != NULL);
  gTesterMock->beepCmd();
}

void Tester::setSingleCmd() {
  assert(gTesterMock != NULL);
  gTesterMock->setSingleCmd();
}

void Tester::setAllCmd() {
  assert(gTesterMock != NULL);
  gTesterMock->setAllCmd();
}

void Tester::readEOLsensorsCmd() {
  assert(gTesterMock != NULL);
  gTesterMock->readEOLsensorsCmd();
}

void Tester::readEncodersCmd() {
  assert(gTesterMock != NULL);
  gTesterMock->readEncodersCmd();
}

void Tester::autoReadCmd() {
  assert(gTesterMock != NULL);
  gTesterMock->autoReadCmd();
}

void Tester::autoTestCmd() {
  assert(gTesterMock != NULL);
  gTesterMock->autoTestCmd();
}

void Tester::stopCmd() {
  assert(gTesterMock != NULL);
  gTesterMock->stopCmd();
}

void Tester::quitCmd() {
  assert(gTesterMock != NULL);
  gTesterMock->quitCmd();
}

void Tester::unrecognizedCmd(const char *buffer) {
  assert(gTesterMock != NULL);
  gTesterMock->unrecognizedCmd(buffer);
}
