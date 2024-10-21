#include "api.h"

#include <string.h>

#include "crc8.h"

// Single knitter instance allowed (list of pointers to support > 1 handler)
API *apiInstance;
void staticRxMessageHandler(const uint8_t *buffer, size_t size) {
  apiInstance->rxMessageHandler(buffer, size);
}

API::API(hardwareAbstraction::HalInterface *hal) {
  _hal = hal;
  _error = ErrorCode::Success;

  // Register Rx callback
  apiInstance = this;
  _hal->packetSerial->setPacketHandler(staticRxMessageHandler);
}

void API::schedule() { _hal->packetSerial->schedule(); }

//----------------------------------------------------------------------------
// RX message handlers
//----------------------------------------------------------------------------

void API::rxMessageHandler(const uint8_t *buffer, size_t size) {
  // Ignore empty packet (python's sliplib starts with empty packets)
  if (size == 0) {
    return;
  }

  _apiRxTrafficIndication();
  _error = ErrorCode::MessageIncorrectLenght;
  switch ((AYAB_API)buffer[0]) {
    case AYAB_API::requestReset:
      // buffer[1] = {crc8}
      if (size == 2) {
        _error = ErrorCode::MessageChecksum;
        if (crc8(buffer, 1) == buffer[1]) {
          _apiRequestReset();
          _error = ErrorCode::Success;
        }
      }
      _apiRxIndicateState();
      break;

    case AYAB_API::requestInit:
      // buffer[1:2] = {machine, crc8}
      if (size == 3) {
        _error = ErrorCode::MessageChecksum;
        if (crc8(buffer, 2) == buffer[2]) {
          _error = _apiRequestInit((MachineType)buffer[1]);
        }
      }
      _apiConfirmInit();
      break;

    case AYAB_API::requestStart:
      // buffer[1:4] = {startNeedle, stopNeedle, continuousReporting, crc8}
      if (size == 5) {
        _error = ErrorCode::MessageChecksum;
        if (crc8(buffer, 4) == buffer[4]) {
          _error = _apiRxSetConfig(buffer[1], buffer[2], (buffer[3] & 1) != 0,
                                   (buffer[3] & 2) != 0);
        }
      }
      _apiConfirmStart();
      break;

    case AYAB_API::confirmLine:
      // buffer[1] = linenumber, buffer[2] = color, buffer[3] = flags (.0=last
      // line) buffer[4:28] = line pattern, buffer[29] = crc8
      if (size <= 30) {
        _error = ErrorCode::MessageChecksum;
        if (crc8(buffer, size-1) == buffer[size-1]) {
          _error = _apiRxSetLine(buffer[1], buffer + 4, size - 5, (buffer[3] & 1) != 0);
        }
      }
      _apiRxIndicateState();
      break;

    case AYAB_API::requestInfo:
      // No arguments nor CRC8
      if (size == 1) {
        _apiConfirmInfo();
      }
      _apiRxIndicateState();
      break;

    case AYAB_API::requestState:
      // buffer[1] = {crc8}
      if (size == 2) {
        _error = ErrorCode::MessageChecksum;
        if (crc8(buffer, 1) == buffer[1]) {
          _error = ErrorCode::Success;
        }
      }
      _apiRxIndicateState();
      break;

    case AYAB_API::debugRequestPoke:
      // buffer[1:3] = {addressLow, addressHigh, value, crc8}
      if (size == 5) {
        _error = ErrorCode::MessageChecksum;
        if (crc8(buffer, 4) == buffer[4]) {
          uint8_t *value = (uint8_t *)((buffer[2] << 8) + buffer[1]);
          *value = buffer[3];
        }
      }
      _apiRxIndicateState();
      break;

    case AYAB_API::debugRequestPeek:
      // buffer[1:3] = {addressLow, addressHigh, crc8}
      if (size == 4) {
        _error = ErrorCode::MessageChecksum;
        if (crc8(buffer, 3) == buffer[3]) {
          uint8_t *value = (uint8_t *)((buffer[2] << 8) + buffer[1]);
          _apiConfirmPeek(*value);
          break;
        }
      }
      _apiRxIndicateState();
      break;

    default:
      _error = ErrorCode::MessageUnknown;
      _apiRxIndicateState();
      break;
  }
}

//----------------------------------------------------------------------------
//  TX message handlers
//----------------------------------------------------------------------------

void API::_apiIndicateState(KnitterState state, uint16_t hallValueLeft,
                            uint16_t hallValueRight, CarriageType carriageType,
                            uint8_t carriagePosition,
                            Direction carriageDirection, MachineSide hallActive,
                            BeltShift beltshift) {
  uint8_t message[] = {(uint8_t)AYAB_API::indicateState,
                       (uint8_t)_error,
                       (uint8_t)state,
                       (uint8_t)((hallValueLeft >> 8) & 0xff),
                       (uint8_t)(hallValueLeft & 0xff),
                       (uint8_t)((hallValueRight >> 8) & 0xff),
                       (uint8_t)(hallValueRight & 0xff),
                       (uint8_t)carriageType,
                       carriagePosition,
                       (uint8_t)carriageDirection,
                       (uint8_t)hallActive,
                       (uint8_t)beltshift,
                       0};
  size_t size = sizeof(message);
  message[size - 1] = crc8(message, size - 1);
  _hal->packetSerial->send(message, size);
  _apiTxTrafficIndication();
}

void API::_apiConfirmInit() {
  uint8_t message[] = {(uint8_t)AYAB_API::confirmInit, (uint8_t)_error, 0};
  size_t size = sizeof(message);
  message[size - 1] = crc8(message, size - 1);
  _hal->packetSerial->send(message, size);
  _apiTxTrafficIndication();
}

void API::_apiConfirmStart() {
  uint8_t message[] = {(uint8_t)AYAB_API::confirmStart, (uint8_t)_error, 0};
  size_t size = sizeof(message);
  message[size - 1] = crc8(message, size - 1);
  _hal->packetSerial->send(message, size);
  _apiTxTrafficIndication();
}

void API::_apiRequestLine(uint8_t rowNumber, ErrorCode error) {
  uint8_t message[] = {(uint8_t)AYAB_API::requestLine, (uint8_t)rowNumber,
                       (uint8_t)error, 0};
  size_t size = sizeof(message);
  message[size - 1] = crc8(message, size - 1);
  _hal->packetSerial->send(message, size);
  _apiTxTrafficIndication();
}

// TODO: Add report of HW variant
void API::_apiConfirmInfo() {
  uint8_t message[22] = {(uint8_t)AYAB_API::confirmInfo, API_VERSION,
                         FW_VERSION_MAJ, FW_VERSION_MIN, FW_VERSION_PATCH};
  size_t size = sizeof(message);
  strncpy((char *)message + 5, FW_VERSION_SUFFIX, 16);
  message[size - 1] = crc8(message, size - 1);
  _hal->packetSerial->send(message, sizeof(message));
  _apiTxTrafficIndication();
}

void API::_apiConfirmPeek(uint8_t returnValue) {
  uint8_t message[] = {(uint8_t)AYAB_API::debugConfirmPeek, returnValue, 0};
  size_t size = sizeof(message);
  message[size - 1] = crc8(message, size - 1);
  _hal->packetSerial->send(message, size);
  _apiTxTrafficIndication();
}