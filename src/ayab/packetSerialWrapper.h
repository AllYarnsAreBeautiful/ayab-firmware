/*!
 * \file packetSerialWrapper.h
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

#ifndef PACKETSERIALWRAPPER_H_
#define PACKETSERIALWRAPPER_H_

#include <Arduino.h>
#include <PacketSerial.h>

class PacketSerialWrapperInterface {
public:
  virtual ~PacketSerialWrapperInterface() = default;

  // any methods that need to be mocked should go here
  virtual void begin(uint32_t speed) = 0;
  virtual void send(const uint8_t *buffer, size_t size) const = 0;
  virtual void setPacketHandler(SLIPPacketSerial::PacketHandlerFunction onPacketFunction) = 0;
  virtual void update() = 0;
};

// Container class for the static method packetSerial.
// Dependency injection is enabled using a pointer to a global instance of
// either `PacketSerialWrapper` or `PacketSerialWrapperMock`,
// both of which classes implement the
// pure virtual methods of `PacketSerialWrapperInterface`.

class GlobalPacketSerialWrapper final {
private:
  // singleton class so private constructor is appropriate
  GlobalPacketSerialWrapper() = default;

public:
  // pointer to global instance whose methods are implemented
  static PacketSerialWrapperInterface *m_instance;

  static void begin(uint32_t speed);
  static void send(const uint8_t *buffer, size_t size);
  static void setPacketHandler(SLIPPacketSerial::PacketHandlerFunction onPacketFunction);
  static void update();
};

/*!
 * \brief Wrapper for packetSerial method
 */
class PacketSerialWrapper : public PacketSerialWrapperInterface {
public:
  void begin(uint32_t speed) final;
  void send(const uint8_t *buffer, size_t size) const final;
  void setPacketHandler(SLIPPacketSerial::PacketHandlerFunction onPacketFunction) final;
  void update() final;

private:
  SLIPPacketSerial m_packetSerial;
};

#endif // PACKETSERIALWRAPPER_H_
