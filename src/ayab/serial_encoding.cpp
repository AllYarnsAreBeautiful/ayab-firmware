// serial_encoding.cpp
/*
This file is part of AYAB.

    AYAB is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    AYAB is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with AYAB.  If not, see <http://www.gnu.org/licenses/>.

    Copyright 2013 Christian Obersteiner, Andreas Müller
    http://ayab-knitting.com
*/

#include <Arduino.h>

#include "knitter.h"
#include "serial_encoding.h"

static uint8_t lineBuffer[25];

extern Knitter *knitter;

/* Serial Command handling */

/*!
 * \brief Handle start request command.
 *
 * \todo sl: Assert size? Handle error?
 */
static void h_reqStart(const uint8_t *buffer, size_t size) {
  if (size < 4) {
    // Need 4 bytes from buffer below.
    return;
  }

  uint8_t _startNeedle = (uint8_t)buffer[1];
  uint8_t _stopNeedle = (uint8_t)buffer[2];
  bool _continuousReportingEnabled = (bool)buffer[3];

  // TODO verify operation
  // memset(lineBuffer,0,sizeof(lineBuffer));
  // temporary solution:
  for (int i = 0; i < 25; i++) {
    lineBuffer[i] = 0xFF;
  }

  bool _success = knitter->startOperation(
      _startNeedle, _stopNeedle, _continuousReportingEnabled, &(lineBuffer[0]));

  uint8_t payload[2];
  payload[0] = cnfStart_msgid;
  payload[1] = _success;
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
  uint8_t crc = 0x00;

  while (len--) {
    uint8_t extract = *buffer;
    buffer++;

    for (uint8_t tempI = 8; tempI; tempI--) {
      uint8_t sum = (crc ^ extract) & 0x01;
      crc >>= 1;

      if (sum) {
        crc ^= 0x8C;
      }
      extract >>= 1;
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
  if (size < 29) {
    // Need 29 bytes from buffer below.
    return;
  }

  uint8_t _lineNumber = buffer[1];

  for (int i = 0; i < 25; i++) {
    // Values have to be inverted because of needle states
    lineBuffer[i] = ~buffer[i + 2];
  }
  uint8_t _flags = buffer[27];

#ifdef AYAB_ENABLE_CRC
  uint8_t _crc8 = buffer[28];
  // Check crc on bytes 0-28 of buffer.
  if (_crc8 != CRC8(buffer, 28)) {
    return;
  }
#endif

  if (knitter->setNextLine(_lineNumber)) {
    // Line was accepted
    bool _flagLastLine = bitRead(_flags, 0);
    if (_flagLastLine) {
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
  bool _success = knitter->startTest();

  uint8_t payload[2];
  payload[0] = cnfTest_msgid;
  payload[1] = _success;
  knitter->send(payload, 2);
}

static void h_unrecognized() {
  return;
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
