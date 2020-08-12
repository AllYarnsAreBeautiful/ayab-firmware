/*!`
 * \file SerialCommand_mock.h
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

#ifndef SERIAL_COMMAND_MOCK_H_
#define SERIAL_COMMAND_MOCK_H_

#include <gmock/gmock.h>

class SerialCommandMock {
public:
  MOCK_METHOD0(next, char *());
  MOCK_METHOD2(addCommand, void(const char *command, void (*function)()));
  MOCK_METHOD1(setDefaultHandler, void(void (*function)(const char *command)));
  MOCK_METHOD0(readSerial, void());
};

SerialCommandMock *serialCommandMockInstance();
void releaseSerialCommandMock();

#endif // SERIAL_COMMAND_MOCK_H_
