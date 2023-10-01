/*!
 * \file packetSerialWrapper.cpp
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

#include "packetSerialWrapper.h"

/*!
 * \brief Wrapper for PacketSerial::begin
 */
void PacketSerialWrapper::begin(uint32_t speed) {
#ifndef AYAB_TESTS
  m_packetSerial.begin(speed);
#else
  (void) speed;
#endif
}

/*!
 * \brief Wrapper for PacketSerial::send
 */
void PacketSerialWrapper::send(const uint8_t *buffer, size_t size) const {
#ifndef AYAB_TESTS
  m_packetSerial.send(buffer, size);
#else
  (void) buffer;
  (void) size;
#endif
}

/*!
 * \brief Wrapper for PacketSerial::setPacketHandler
 */
void PacketSerialWrapper::setPacketHandler(SLIPPacketSerial::PacketHandlerFunction onPacketFunction) {
#ifndef AYAB_TESTS
  m_packetSerial.setPacketHandler(onPacketFunction);
#else
  (void) onPacketFunction;
#endif
}

/*!
 * \brief Wrapper for PacketSerial::update
 */
void PacketSerialWrapper::update() {
#ifndef AYAB_TESTS
  m_packetSerial.update();
#endif
}
