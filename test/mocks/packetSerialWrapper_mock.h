/*!`
 * \file packetSerialWrapper_mock.h
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

#ifndef PACKETSERIALWRAPPER_MOCK_H_
#define PACKETSERIALWRAPPER_MOCK_H_

#include <gmock/gmock.h>

#include <packetSerialWrapper.h>

class PacketSerialWrapperMock : public PacketSerialWrapperInterface {
public:
  MOCK_METHOD1(begin, void(uint32_t speed));
  MOCK_CONST_METHOD2(send, void(const uint8_t *buffer, size_t size));
  MOCK_METHOD1(setPacketHandler, void(SLIPPacketSerial::PacketHandlerFunction onPacketFunction));
  MOCK_METHOD0(update, void());
};

PacketSerialWrapperMock *packetSerialWrapperMockInstance();
void releasePacketSerialWrapperMock();

#endif // PACKETSERIALWRAPPER_MOCK_H_
