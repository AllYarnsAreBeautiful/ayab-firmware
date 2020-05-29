#include <Arduino.h>

#include "knitter.h"

static byte lineBuffer[25];

extern Knitter *knitter;

/*
 * Serial Command handling
 */
static void h_reqStart(const uint8_t *buffer, size_t size) {
  byte _startNeedle = (byte)buffer[1];
  byte _stopNeedle = (byte)buffer[2];
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

static void h_cnfLine(const uint8_t *buffer, size_t size) {
  byte _lineNumber = 0;
  byte _flags = 0;
  byte _crc8 = 0;
  bool _flagLastLine = false;

  _lineNumber = (byte)buffer[1];

  for (int i = 0; i < 25; i++) {
    // Values have to be inverted because of needle states
    lineBuffer[i] = ~(byte)buffer[i + 2];
  }
  _flags = (byte)buffer[27];
  _crc8 = (byte)buffer[28];

  // TODO insert CRC8 check

  if (knitter->setNextLine(_lineNumber)) {
    // Line was accepted
    _flagLastLine = bitRead(_flags, 0);
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
