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

#include "encoders.h"
#include "fsm.h"

#ifndef AYAB_TESTS
  #include "version.h"
#else
  constexpr uint8_t FW_VERSION_MAJ = 0U;
  constexpr uint8_t FW_VERSION_MIN = 0U;
  constexpr uint8_t FW_VERSION_PATCH = 0U;
  constexpr char FW_VERSION_SUFFIX[] = "";
#endif // AYAB_TESTS

constexpr uint8_t API_VERSION = 6U;

constexpr uint32_t SERIAL_BAUDRATE = 115200U;

constexpr uint8_t MAX_LINE_BUFFER_LEN = 25U;
constexpr uint8_t MAX_MSG_BUFFER_LEN = 255U;

enum AYAB_API {
  reqStart_msgid = 0x01,
  cnfStart_msgid = 0xC1,
  reqLine_msgid = 0x82,
  cnfLine_msgid = 0x42,
  reqInfo_msgid = 0x03,
  cnfInfo_msgid = 0xC3,
  reqTest_msgid = 0x04,
  cnfTest_msgid = 0xC4,
  indState_msgid = 0x84,
  helpCmd_msgid = 0x25,
  sendCmd_msgid = 0x26,
  beepCmd_msgid = 0x27,
  setSingleCmd_msgid = 0x28,
  setAllCmd_msgid = 0x29,
  readEOLsensorsCmd_msgid = 0x2A,
  readEncodersCmd_msgid = 0x2B,
  autoReadCmd_msgid = 0x2C,
  autoTestCmd_msgid = 0x2D,
  stopCmd_msgid = 0x2E,
  quitCmd_msgid = 0x2F,
  reqInit_msgid = 0x05,
  cnfInit_msgid = 0xC5,
  testRes_msgid = 0xEE,
  debug_msgid = 0x9F
};
using AYAB_API_t = enum AYAB_API;

// API constants
constexpr uint8_t INDSTATE_LEN = 10U;
constexpr uint8_t REQLINE_LEN = 3U;

class ComInterface {
public:
  virtual ~ComInterface() = default;

  // any methods that need to be mocked should go here
  virtual void init() = 0;
  virtual void update() = 0;
  virtual void send(uint8_t *payload, size_t length) const = 0;
  virtual void sendMsg(AYAB_API_t id, const char *msg) = 0;
  virtual void sendMsg(AYAB_API_t id, char *msg) = 0;
  virtual void send_reqLine(const uint8_t lineNumber,
                            Err_t error = ErrorCode::SUCCESS) const = 0;
  virtual void send_indState(Carriage_t carriage, uint8_t position,
                             Err_t error = ErrorCode::SUCCESS) const = 0;
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
  static void send_reqLine(const uint8_t lineNumber, Err_t error = ErrorCode::SUCCESS);
  static void send_indState(Carriage_t carriage, uint8_t position,
                             Err_t error = ErrorCode::SUCCESS);
  static void onPacketReceived(const uint8_t *buffer, size_t size);

private:
  static SLIPPacketSerial m_packetSerial;
};

class Com : public ComInterface {
public:
  void init() final;
  void update() final;
  void send(uint8_t *payload, size_t length) const final;
  void sendMsg(AYAB_API_t id, const char *msg) final;
  void sendMsg(AYAB_API_t id, char *msg) final;
  void send_reqLine(const uint8_t lineNumber, Err_t error = ErrorCode::SUCCESS) const final;
  void send_indState(Carriage_t carriage, uint8_t position,
                             Err_t error = ErrorCode::SUCCESS) const final;
  void onPacketReceived(const uint8_t *buffer, size_t size) final;

private:
  SLIPPacketSerial m_packetSerial;
  uint8_t lineBuffer[MAX_LINE_BUFFER_LEN] = {0};
  uint8_t msgBuffer[MAX_MSG_BUFFER_LEN] = {0};

  void h_reqInit(const uint8_t *buffer, size_t size);
  void h_reqStart(const uint8_t *buffer, size_t size);
  void h_cnfLine(const uint8_t *buffer, size_t size);
  void h_reqInfo() const;
  void h_reqTest(const uint8_t *buffer, size_t size) const;
  void h_unrecognized();

  void send_cnfInfo() const;
  void send_cnfInit(Err_t error) const;
  void send_cnfStart(Err_t error) const;
  void send_cnfTest(Err_t error) const;
  uint8_t CRC8(const uint8_t *buffer, size_t len) const;
};

#endif // COM_H_
