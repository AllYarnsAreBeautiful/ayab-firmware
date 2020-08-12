/*!`
 * \file SerialCommand_mock.cpp
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

#include <SerialCommand.h>
#include <SerialCommand_mock.h>

static SerialCommandMock *gSerialCommandMock = NULL;
SerialCommandMock *serialCommandMockInstance() {
  if (!gSerialCommandMock) {
    gSerialCommandMock = new SerialCommandMock();
  }
  return gSerialCommandMock;
}

void releaseSerialCommandMock() {
  if (gSerialCommandMock) {
    delete gSerialCommandMock;
    gSerialCommandMock = NULL;
  }
}

SerialCommand::SerialCommand() {
}

char *SerialCommand::next() {
  assert(gSerialCommandMock != nullptr);
  return gSerialCommandMock->next();
}

void SerialCommand::addCommand(const char *command, void (*function)()) {
  assert(gSerialCommandMock != nullptr);
  gSerialCommandMock->addCommand(command, function);
}

void SerialCommand::setDefaultHandler(void (*function)(const char *command)) {
  assert(gSerialCommandMock != nullptr);
  gSerialCommandMock->setDefaultHandler(function);
}

void SerialCommand::readSerial() {
  assert(gSerialCommandMock != nullptr);
  gSerialCommandMock->readSerial();
}
