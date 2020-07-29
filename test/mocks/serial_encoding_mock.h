/*!`
 * \file serial_encoding_mock.h
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

#ifndef SERIAL_ENCODING_MOCK_H_
#define SERIAL_ENCODING_MOCK_H_

#include <gmock/gmock.h>

class SerialEncodingMock {
public:
  MOCK_METHOD0(update, void());
  MOCK_METHOD2(send, void(uint8_t *payload, size_t length));
  MOCK_METHOD2(onPacketReceived, void(const uint8_t *buffer, size_t size));
};

SerialEncodingMock *serialEncodingMockInstance();
void releaseSerialEncodingMock();

#endif  // SERIAL_ENCODING_MOCK_H_
