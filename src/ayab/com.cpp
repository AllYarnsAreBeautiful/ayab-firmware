/*!
 * \file com.cpp
 * \brief Class containing methods for serial communication.
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

#include "com.h"
#include "knitter.h"
#include "tester.h"

/*!
 * \brief Initialize serial communication.
 */
void Com::init() {
  m_packetSerial.begin(SERIAL_BAUDRATE);
#ifndef AYAB_TESTS
  m_packetSerial.setPacketHandler(GlobalCom::onPacketReceived);
#endif // AYAB_TESTS
}

/*!
 * \brief Service the serial connection.
 */
void Com::update() {
  m_packetSerial.update();
}

/*!
 * \brief Send a packet of data.
 * \param payload A pointer to a data buffer.
 * \param length The number of bytes in the data buffer.
 */
void Com::send(uint8_t *payload, size_t length) const {
  // TODO(TP): insert a workaround for hardware test code
  /*
  #ifdef AYAB_HW_TEST
    Serial.print("Sent: ");
    for (uint8_t i = 0; i < length; ++i) {
      Serial.print(payload[i]);
    }
    Serial.print(", Encoded as: ");
  #endif
  */
  m_packetSerial.send(payload, length);
}

/*!
 * \brief send initial msgid followed by null-terminated string
 * \param id The msgid to be sent.
 * \param msg Pointer to a data buffer containing a null-terminated string.
 */
void Com::sendMsg(AYAB_API_t id, const char *msg) {
  uint8_t length = 0;
  msgBuffer[length++] = static_cast<uint8_t>(id);
  while (*msg) {
    msgBuffer[length++] = static_cast<uint8_t>(*msg++);
  }
  m_packetSerial.send(msgBuffer, length);
}
void Com::sendMsg(AYAB_API_t id, char *msg) {
  sendMsg(id, static_cast<const char *>(msg));
}

/*!
 * \brief Send `reqLine` message.
 * \param lineNumber The line number requested (0-indexed and modulo 256).
 * \param error Error code (0 = success).
 */
void Com::send_reqLine(const uint8_t lineNumber, Err_t error) const {
  // `payload` will be allocated on stack since length is compile-time constant
  uint8_t payload[REQLINE_LEN] = {static_cast<uint8_t>(AYAB_API::reqLine), lineNumber, static_cast<uint8_t>(error)};
  send(static_cast<uint8_t *>(payload), REQLINE_LEN);
}

/*!
 * \brief Send `indState` message.
 * \param carriage Type of knitting carriage in use.
 * \param position Position of knitting carriage in needles from left hand side.
 * \param initState State of readiness (0 = ready, other values = not ready).
 */
void Com::send_indState(Carriage_t carriage, uint8_t position,
                        Err_t error) const {
  uint16_t leftHallValue = GlobalEncoders::getHallValue(Direction_t::Left);
  uint16_t rightHallValue = GlobalEncoders::getHallValue(Direction_t::Right);
  // `payload` will be allocated on stack since length is compile-time constant
  uint8_t payload[INDSTATE_LEN] = {
      static_cast<uint8_t>(AYAB_API::indState),
      static_cast<uint8_t>(error),
      static_cast<uint8_t>(GlobalFsm::getState()),
      highByte(leftHallValue),
      lowByte(leftHallValue),
      highByte(rightHallValue),
      lowByte(rightHallValue),
      static_cast<uint8_t>(carriage),
      position,
      static_cast<uint8_t>(GlobalEncoders::getDirection()),
  };
  send(static_cast<uint8_t *>(payload), INDSTATE_LEN);
}

/*!
 * \brief Callback for PacketSerial.
 * \param buffer A pointer to a data buffer.
 * \param size The number of bytes in the data buffer.
 */
void Com::onPacketReceived(const uint8_t *buffer, size_t size) {
  switch (buffer[0]) {
  case static_cast<uint8_t>(AYAB_API::reqInit):
    h_reqInit(buffer, size);
    break;

  case static_cast<uint8_t>(AYAB_API::reqStart):
    h_reqStart(buffer, size);
    break;

  case static_cast<uint8_t>(AYAB_API::cnfLine):
    h_cnfLine(buffer, size);
    break;

  case static_cast<uint8_t>(AYAB_API::reqInfo):
    h_reqInfo();
    break;

  case static_cast<uint8_t>(AYAB_API::reqTest):
    h_reqTest();
    break;

  case static_cast<uint8_t>(AYAB_API::helpCmd):
    GlobalTester::helpCmd();
    break;

  case static_cast<uint8_t>(AYAB_API::sendCmd):
    GlobalTester::sendCmd();
    break;

  case static_cast<uint8_t>(AYAB_API::beepCmd):
    GlobalTester::beepCmd();
    break;

  case static_cast<uint8_t>(AYAB_API::setSingleCmd):
    GlobalTester::setSingleCmd(buffer, size);
    break;

  case static_cast<uint8_t>(AYAB_API::setAllCmd):
    GlobalTester::setAllCmd(buffer, size);
    break;

  case static_cast<uint8_t>(AYAB_API::readEOLsensorsCmd):
    GlobalTester::readEOLsensorsCmd();
    break;

  case static_cast<uint8_t>(AYAB_API::readEncodersCmd):
    GlobalTester::readEncodersCmd();
    break;

  case static_cast<uint8_t>(AYAB_API::autoReadCmd):
    GlobalTester::autoReadCmd();
    break;

  case static_cast<uint8_t>(AYAB_API::autoTestCmd):
    GlobalTester::autoTestCmd();
    break;

  case static_cast<uint8_t>(AYAB_API::stopCmd):
    GlobalTester::stopCmd();
    break;

   case static_cast<uint8_t>(AYAB_API::quitCmd):
    GlobalTester::quitCmd();
    break;

  default:
    h_unrecognized();
    break;
  }
}

// Serial command handling

/*!
 * \brief Handle `reqInit` (request initialization) command.
 * \param buffer A pointer to a data buffer.
 * \param size The number of bytes in the data buffer.
 */
void Com::h_reqInit(const uint8_t *buffer, size_t size) {
  if (size < 3U) {
    // Need 3 bytes from buffer below.
    send_cnfInit(ErrorCode::expected_longer_message);
    return;
  }

  auto machineType = static_cast<Machine_t>(buffer[1]);

  uint8_t crc8 = buffer[2];
  // Check crc on bytes 0-4 of buffer.
  if (crc8 != CRC8(buffer, 2)) {
    send_cnfInit(ErrorCode::checksum_error);
    return;
  }

  memset(lineBuffer, 0xFF, MAX_LINE_BUFFER_LEN);

  Err_t error = GlobalKnitter::initMachine(machineType);
  send_cnfInit(error);
}

/*!
 * \brief Handle `reqStart` (start request) command.
 * \param buffer A pointer to a data buffer.
 * \param size The number of bytes in the data buffer.
 */
void Com::h_reqStart(const uint8_t *buffer, size_t size) {
  if (size < 5U) {
    // Need 5 bytes from buffer below.
    send_cnfStart(ErrorCode::expected_longer_message);
    return;
  }

  uint8_t startNeedle = buffer[1];
  uint8_t stopNeedle = buffer[2];
  auto continuousReportingEnabled = static_cast<bool>(buffer[3] & 1);
  auto beeperEnabled = static_cast<bool>(buffer[3] & 2);

  uint8_t crc8 = buffer[4];
  // Check crc on bytes 0-4 of buffer.
  if (crc8 != CRC8(buffer, 4)) {
    send_cnfStart(ErrorCode::checksum_error);
    return;
  }

  GlobalBeeper::init(beeperEnabled);
  memset(lineBuffer, 0xFF, MAX_LINE_BUFFER_LEN);

  // Note (August 2020): the return value of this function has changed.
  // Previously, it returned `true` for success and `false` for failure.
  // Now, it returns `0` for success and an informative error code otherwise.
  Err_t error =
      GlobalKnitter::startKnitting(startNeedle, stopNeedle,
                                   lineBuffer, continuousReportingEnabled);
  send_cnfStart(error);
}

/*!
 * \brief Handle `cnfLine` (configure line) command.
 * \param buffer A pointer to a data buffer.
 * \param size The number of bytes in the data buffer.
 *
 * \todo sl: Handle CRC-8 error?
 * \todo sl: Assert size? Handle error?
 */
void Com::h_cnfLine(const uint8_t *buffer, size_t size) {
  auto machineType = static_cast<uint8_t>(GlobalKnitter::getMachineType());
  uint8_t lenLineBuffer = LINE_BUFFER_LEN[machineType];
  if (size < lenLineBuffer + 5U) {
    // message is too short
    // TODO(sl): handle error?
    // TODO(TP): send repeat request with error code?
    return;
  }

  uint8_t lineNumber = buffer[1];
  /* uint8_t color = buffer[2];  */ // currently unused
  uint8_t flags = buffer[3];

  for (uint8_t i = 0U; i < lenLineBuffer; i++) {
    // Values have to be inverted because of needle states
    lineBuffer[i] = ~buffer[i + 4];
  }

  uint8_t crc8 = buffer[lenLineBuffer + 4];
  // Calculate checksum of buffer contents
  if (crc8 != CRC8(buffer, lenLineBuffer + 4)) {
    // TODO(sl): handle checksum error?
    // TODO(TP): send repeat request with error code?
    return;
  }

  if (GlobalKnitter::setNextLine(lineNumber)) {
    // Line was accepted
    bool flagLastLine = bitRead(flags, 0U);
    if (flagLastLine) {
      GlobalKnitter::setLastLine();
    }
  }
}

/*!
 * \brief Handle `reqInfo` (request information) command.
 * \param buffer A pointer to a data buffer.
 * \param size The number of bytes in the data buffer.
 */
void Com::h_reqInfo() const {
  send_cnfInfo();
}

/*!
 * \brief Handle `reqTest` (request hardware test) command.
 * \param buffer A pointer to a data buffer.
 * \param size The number of bytes in the data buffer.
 */
void Com::h_reqTest() const {
  auto machineType = static_cast<Machine_t>(GlobalKnitter::getMachineType());

  // Note (August 2020): the return value of this function has changed.
  // Previously, it returned `true` for success and `false` for failure.
  // Now, it returns `0` for success and an informative error code otherwise.
  Err_t error = GlobalTester::startTest(machineType);
  send_cnfTest(error);
}

// GCOVR_EXCL_START
/*!
 * \brief Handle unrecognized command.
 */
void Com::h_unrecognized() const {
  // do nothing
}
// GCOVR_EXCL_STOP

/*!
 * \brief Send `cnfInfo` message.
 */
void Com::send_cnfInfo() const {
  // Max. length of suffix string: 16 bytes + \0
  // `payload` will be allocated on stack since length is compile-time constant
  uint8_t payload[22];
  payload[0] = static_cast<uint8_t>(AYAB_API::cnfInfo);
  payload[1] = API_VERSION;
  payload[2] = FW_VERSION_MAJ;
  payload[3] = FW_VERSION_MIN;
  payload[4] = FW_VERSION_PATCH;
  strncpy((char*)payload + 5, FW_VERSION_SUFFIX, 16);
  send(payload, 22);
}

/*!
 * \brief Send `cnfInit` message.
 * \param error Error code (0 = success, other values = error).
 */
void Com::send_cnfInit(Err_t error) const {
  // `payload` will be allocated on stack since length is compile-time constant
  uint8_t payload[2];
  payload[0] = static_cast<uint8_t>(AYAB_API::cnfInit);
  payload[1] = static_cast<uint8_t>(error);
  send(payload, 2);
}


/*!
 * \brief Send `cnfStart` message.
 * \param error Error code (0 = success, other values = error).
 */
void Com::send_cnfStart(Err_t error) const {
  // `payload` will be allocated on stack since length is compile-time constant
  uint8_t payload[2];
  payload[0] = static_cast<uint8_t>(AYAB_API::cnfStart);
  payload[1] = static_cast<uint8_t>(error);
  send(payload, 2);
}

/*!
 * \brief Send `cnfTest` message.
 * \param error Error code (0 = success, other values = error).
 */
void Com::send_cnfTest(Err_t error) const {
  // `payload` will be allocated on stack since length is compile-time constant
  uint8_t payload[2];
  payload[0] = static_cast<uint8_t>(AYAB_API::cnfTest);
  payload[1] = static_cast<uint8_t>(error);
  send(payload, 2);
}

/*!
 * \brief Calculate CRC8 of a buffer.
 * \param buffer A pointer to a data buffer.
 * \param len The number of bytes of data in the data buffer.
 *
 * Based on
 * https://www.leonardomiliani.com/en/2013/un-semplice-crc8-per-arduino/
 *
 * CRC-8 - based on the CRC8 formulas by Dallas/Maxim
 * code released under the therms of the GNU GPL 3.0 license
 *
 * Faster code using a lookup table is available, if needed.
 */
uint8_t Com::CRC8(const uint8_t *buffer, size_t len) const {
  uint8_t crc = 0x00U;

  while (len--) {
    uint8_t extract = *buffer;
    buffer++;

    for (uint8_t tempI = 8U; tempI; tempI--) {
      uint8_t sum = (crc ^ extract) & 0x01U;
      crc >>= 1U;

      if (sum) {
        crc ^= 0x8CU;
      }
      extract >>= 1U;
    }
  }
  return crc;
}

