/*!
 * \file com.h
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

#ifndef COM_H_
#define COM_H_

#include <Arduino.h>
#include <PacketSerial.h>

constexpr uint8_t FW_VERSION_MAJ = 1U;
constexpr uint8_t FW_VERSION_MIN = 0U;
constexpr uint8_t FW_VERSION_PATCH = 0U;

constexpr uint8_t API_VERSION = 6U;

constexpr uint32_t SERIAL_BAUDRATE = 115200U;

constexpr uint8_t MAX_LINE_BUFFER_LEN = 25U;
constexpr uint8_t MAX_MSG_BUFFER_LEN = 255U;

enum AYAB_API {
  reqStart_msgid = 0x01,
  cnfStart_msgid = 0xc1,
  reqLine_msgid = 0x82,
  cnfLine_msgid = 0x42,
  reqInfo_msgid = 0x03,
  cnfInfo_msgid = 0xc3,
  reqTest_msgid = 0x04,
  cnfTest_msgid = 0xc4,
  indState_msgid = 0x84,
  helpCmd_msgid = 0x25,
  sendCmd_msgid = 0x26,
  beepCmd_msgid = 0x27,
  setSingleCmd_msgid = 0x28,
  setAllCmd_msgid = 0x29,
  readEOLsensorsCmd_msgid = 0x2a,
  readEncodersCmd_msgid = 0x2b,
  autoReadCmd_msgid = 0x2c,
  autoTestCmd_msgid = 0x2d,
  stopCmd_msgid = 0x2e,
  quitCmd_msgid = 0x2f,
  testRes_msgid = 0xe0,
  debug_msgid = 0x99
};
using AYAB_API_t = enum AYAB_API;

class ComInterface {
public:
  virtual ~ComInterface(){};

  // any methods that need to be mocked should go here
  virtual void init() = 0;
  virtual void update() = 0;
  virtual void send(uint8_t *payload, size_t length) = 0;
  virtual void sendMsg(AYAB_API_t id, const char *msg) = 0;
  virtual void sendMsg(AYAB_API_t id, char *msg) = 0;
  virtual void onPacketReceived(const uint8_t *buffer, size_t size) = 0;
};

// Container class for the static methods that implement the serial API.
// Dependency injection is enabled using a pointer to a global instance of
// either `Com` or `ComMock`, both of which classes implement the
// pure virtual methods of `ComInterface`.

class GlobalCom final {
private:
  // singleton class so private constructor is appropriate
  GlobalCom() = default;

public:
  // pointer to global instance whose methods are implemented
  static ComInterface *m_instance;

  static void init();
  static void update();
  static void send(uint8_t *payload, size_t length);
  static void sendMsg(AYAB_API_t id, const char *msg);
  static void sendMsg(AYAB_API_t id, char *msg);
  static void onPacketReceived(const uint8_t *buffer, size_t size);

private:
  static SLIPPacketSerial m_packetSerial;
};

class Com : public ComInterface {
public:
  void init();
  void update();
  void send(uint8_t *payload, size_t length);
  void sendMsg(AYAB_API_t id, const char *msg);
  void sendMsg(AYAB_API_t id, char *msg);
  void onPacketReceived(const uint8_t *buffer, size_t size);

private:
  SLIPPacketSerial m_packetSerial;
  uint8_t lineBuffer[MAX_LINE_BUFFER_LEN] = {0};
  uint8_t msgBuffer[MAX_MSG_BUFFER_LEN] = {0};

  void h_reqStart(const uint8_t *buffer, size_t size);
  void h_cnfLine(const uint8_t *buffer, size_t size);
  void h_reqInfo();
  void h_reqTest(const uint8_t *buffer, size_t size);
  void h_unrecognized();
};

#endif // COM_H_
