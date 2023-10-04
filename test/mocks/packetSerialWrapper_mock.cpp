/*!`
 * \file packetSerialWrapper_mock.cpp
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

#include <packetSerialWrapper_mock.h>

static PacketSerialWrapperMock *gPacketSerialWrapperMock = nullptr;
PacketSerialWrapperMock *packetSerialWrapperMockInstance() {
  if (!gPacketSerialWrapperMock) {
    gPacketSerialWrapperMock = new PacketSerialWrapperMock();
  }
  return gPacketSerialWrapperMock;
}

void releasePacketSerialWrapperMock() {
  if (gPacketSerialWrapperMock) {
    delete gPacketSerialWrapperMock;
    gPacketSerialWrapperMock = nullptr;
  }
}

void PacketSerialWrapper::begin(uint32_t speed) {
  assert(gPacketSerialWrapperMock != nullptr);
  gPacketSerialWrapperMock->begin(speed);
}

void PacketSerialWrapper::send(const uint8_t *buffer, size_t size) const {
  assert(gPacketSerialWrapperMock != nullptr);
  gPacketSerialWrapperMock->send(buffer, size);
}

void PacketSerialWrapper::setPacketHandler(SLIPPacketSerial::PacketHandlerFunction onPacketFunction) {
  assert(gPacketSerialWrapperMock != nullptr);
  gPacketSerialWrapperMock->setPacketHandler(onPacketFunction);
}

void PacketSerialWrapper::update() {
  assert(gPacketSerialWrapperMock != nullptr);
  gPacketSerialWrapperMock->update();
}
