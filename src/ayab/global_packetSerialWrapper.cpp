/*!
 * \file global_packetSerialWrapper.cpp
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

// static member functions

void GlobalPacketSerialWrapper::begin(uint32_t speed) {
  m_instance.begin(speed);
}

void GlobalPacketSerialWrapper::send(const uint8_t *buffer, size_t size) {
  m_instance.send(buffer, size);
}

void GlobalPacketSerialWrapper::setPacketHandler(SLIPPacketSerial::PacketHandlerFunction onPacketFunction) {
  m_instance.setPacketHandler(onPacketFunction);
}

void GlobalPacketSerialWrapper::update() {
  m_instance.update();
}
