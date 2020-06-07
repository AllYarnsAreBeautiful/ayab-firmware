/*!
 * \file serial_encoding.cpp
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
 *    Copyright 2013 Christian Obersteiner, Andreas Müller
 *    http://ayab-knitting.com
 */

#include <Arduino.h>

#include "knitter.h"
#include "serial_encoding.h"

static uint8_t lineBuffer[LINEBUFFER_LEN];

extern Knitter *knitter;

/* Serial Command handling */

/*!
 * \brief Handle start request command.
 *
 * \todo sl: Assert size? Handle error?
 */
static void h_reqStart(const uint8_t *buffer, size_t size) {
  if (size < 4U) {
    // Need 4 bytes from buffer below.
    return;
  }

  uint8_t startNeedle = buffer[1];
  uint8_t stopNeedle = buffer[2];
  bool continuousReportingEnabled = static_cast<bool>(buffer[3]);

  // TODO(who?): verify operation
  // memset(lineBuffer,0,sizeof(lineBuffer));
  // temporary solution:
  for (uint8_t i = 0U; i < LINEBUFFER_LEN; i++) {
    lineBuffer[i] = 0xFFU;
  }

  bool success = knitter->startOperation(
      startNeedle, stopNeedle, continuousReportingEnabled, &(lineBuffer[0]));

  uint8_t payload[2];
  payload[0] = cnfStart_msgid;
  payload[1] = static_cast<uint8_t>(success);
  knitter->send(payload, 2);
}

#ifdef AYAB_ENABLE_CRC
/*!
 * \brief Calculate CRC8 of a buffer
 *
 * Based on
 * https://www.leonardomiliani.com/en/2013/un-semplice-crc8-per-arduino/
 *
 * CRC-8 - based on the CRC8 formulas by Dallas/Maxim
 * code released under the therms of the GNU GPL 3.0 license
 */
static uint8_t CRC8(const uint8_t *buffer, size_t len) {
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
#endif

/*!
 * \brief Handle configure line command.
 *
 * \todo sl: Handle CRC-8 error?
 * \todo sl: Assert size? Handle error?
 */
static void h_cnfLine(const uint8_t *buffer, size_t size) {
  if (size < 29U) {
    // Need 29 bytes from buffer below.
    return;
  }

  uint8_t lineNumber = buffer[1];

  for (uint8_t i = 0U; i < LINEBUFFER_LEN; i++) {
    // Values have to be inverted because of needle states
    lineBuffer[i] = ~buffer[i + 2];
  }
  uint8_t flags = buffer[27];

#ifdef AYAB_ENABLE_CRC
  uint8_t crc8 = buffer[28];
  // Check crc on bytes 0-28 of buffer.
  if (crc8 != CRC8(buffer, 28)) {
    return;
  }
#endif

  if (knitter->setNextLine(lineNumber)) {
    // Line was accepted
    bool flagLastLine = bitRead(flags, 0U);
    if (flagLastLine) {
      knitter->setLastLine();
    }
  }
}

static void h_reqInfo() {
  uint8_t payload[4];
  payload[0] = cnfInfo_msgid;
  payload[1] = API_VERSION;
  payload[2] = FW_VERSION_MAJ;
  payload[3] = FW_VERSION_MIN;
  knitter->send(payload, 4);
}

static void h_reqTest() {
  bool success = knitter->startTest();

  uint8_t payload[2];
  payload[0] = cnfTest_msgid;
  payload[1] = static_cast<uint8_t>(success);
  knitter->send(payload, 2);
}

static void h_unrecognized() {
}

/*! Callback for PacketSerial
 *
 */
void onPacketReceived(const uint8_t *buffer, size_t size) {
  switch (buffer[0]) {
  case reqStart_msgid:
    h_reqStart(buffer, size);
    break;

  case cnfLine_msgid:
    h_cnfLine(buffer, size);
    break;

  case reqInfo_msgid:
    h_reqInfo();
    break;

  case reqTest_msgid:
    h_reqTest();
    break;

  default:
    h_unrecognized();
    break;
  }
}

SerialEncoding::SerialEncoding() {
  m_packetSerial.begin(SERIAL_BAUDRATE);
  m_packetSerial.setPacketHandler(&onPacketReceived);
}

void SerialEncoding::update() {
  m_packetSerial.update();
}

void SerialEncoding::send(uint8_t *payload, size_t length) {
  m_packetSerial.send(payload, length);
}
