/*!`
 * \file hw_test_mock.cpp
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

#include <hw_test_mock.h>

static HardwareTestMock *gHardwareTestMock = NULL;
HardwareTestMock *hardwareTestMockInstance() {
  if (!gHardwareTestMock) {
    gHardwareTestMock = new HardwareTestMock();
  }
  return gHardwareTestMock;
}

void releaseHardwareTestMock() {
  if (gHardwareTestMock) {
    delete gHardwareTestMock;
    gHardwareTestMock = NULL;
  }
}

void HardwareTest::helpCmd() {
  assert(gHardwareTestMock != NULL);
  gHardwareTestMock->helpCmd();
}

void HardwareTest::sendCmd() {
  assert(gHardwareTestMock != NULL);
  gHardwareTestMock->sendCmd();
}

void HardwareTest::beepCmd() {
  assert(gHardwareTestMock != NULL);
  gHardwareTestMock->beepCmd();
}

void HardwareTest::setSingleCmd() {
  assert(gHardwareTestMock != NULL);
  gHardwareTestMock->setSingleCmd();
}

void HardwareTest::setAllCmd() {
  assert(gHardwareTestMock != NULL);
  gHardwareTestMock->setAllCmd();
}

void HardwareTest::readEOLsensorsCmd() {
  assert(gHardwareTestMock != NULL);
  gHardwareTestMock->readEOLsensorsCmd();
}

void HardwareTest::readEncodersCmd() {
  assert(gHardwareTestMock != NULL);
  gHardwareTestMock->readEncodersCmd();
}

void HardwareTest::autoReadCmd() {
  assert(gHardwareTestMock != NULL);
  gHardwareTestMock->autoReadCmd();
}

void HardwareTest::autoTestCmd() {
  assert(gHardwareTestMock != NULL);
  gHardwareTestMock->autoTestCmd();
}

void HardwareTest::stopCmd() {
  assert(gHardwareTestMock != NULL);
  gHardwareTestMock->stopCmd();
}

void HardwareTest::quitCmd() {
  assert(gHardwareTestMock != NULL);
  gHardwareTestMock->quitCmd();
}

void HardwareTest::unrecognizedCmd(const char *buffer) {
  assert(gHardwareTestMock != NULL);
  gHardwareTestMock->unrecognizedCmd(buffer);
}

void HardwareTest::setUp() {
  assert(gHardwareTestMock != NULL);
  gHardwareTestMock->setUp();
}

void HardwareTest::loop() {
  assert(gHardwareTestMock != NULL);
  gHardwareTestMock->loop();
}
