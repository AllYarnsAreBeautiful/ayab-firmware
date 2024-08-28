#include "api.h"

// Single knitter instance allowed (list of pointers to support > 1 handler)
API *apiInstance;
void staticRxMessageHandler(const uint8_t *buffer, size_t size) {
  apiInstance->rxMessageHandler(buffer, size);
}

API::API(hardwareAbstraction::HalInterface *hal) {
  _hal = hal;

  // Register Rx callback
  apiInstance = this;
  _hal->packetSerial->setPacketHandler(staticRxMessageHandler);
}

void API::schedule() { _hal->packetSerial->schedule(); }

//----------------------------------------------------------------------------
// RX message handlers
//----------------------------------------------------------------------------

void API::rxMessageHandler(const uint8_t *buffer, size_t size) {
  bool success = false;
  uint8_t *value;

  _apiRxTrafficIndication();

  switch ((AYAB_API)buffer[0]) {
    case AYAB_API::requestReset:
      _apiRequestReset();
      break;
    case AYAB_API::requestStart:
      if (size == 4) {
        // buffer[1:3] = {startNeedle, stopNeedle, continuousReporting}
        success = _apiRxSetConfig(buffer[1], buffer[2], buffer[3] != 0);
      }
      _apiConfirmStart(success);
      break;

    case AYAB_API::confirmLine:
      if (size == 29) {
        // buffer[1] = linenumber, buffer[2:26] = line pattern,
        // buffer[27]=flags (.0=last line), buffer[28]=crc8
        // TODO CRC8 check
        success = _apiRxSetLine(buffer[1], buffer + 2, (buffer[27] & 1) != 0);
      }
      // TODO handle success = false/no feedback to host ?
      break;

    case AYAB_API::requestInfo:
      _apiConfirmInfo();
      break;

    case AYAB_API::requestState:
      _apiRxIndicateState();
      break;

    case AYAB_API::debugRequestPoke:
      value = (uint8_t *)((buffer[2] << 8) + buffer[1]);
      *value = buffer[3];
      break;

    case AYAB_API::debugRequestPeek:
      value = (uint8_t *)((buffer[2] << 8) + buffer[1]);
      _apiConfirmPeek(*value);
      break;
      
    default:
      break;
  }
}

//----------------------------------------------------------------------------
//  TX message handlers
//----------------------------------------------------------------------------

void API::_apiIndicateState(uint16_t hallValueLeft, uint16_t hallValueRight,
                            bool initDone, CarriageType carriageType,
                            uint8_t carriagePosition,
                            Direction carriageDirection,
                            Direction hallActive,
                            BeltShift beltshift) {
  uint8_t message[] = {(uint8_t)AYAB_API::indicateState,
                       (uint8_t)(initDone),
                       (uint8_t)(hallValueLeft & 0xff),
                       (uint8_t)((hallValueLeft >> 8) & 0xff),
                       (uint8_t)(hallValueRight & 0xff),
                       (uint8_t)((hallValueRight >> 8) & 0xff),
                       (uint8_t)carriageType,
                       carriagePosition,
                       (uint8_t)carriageDirection,
                       (uint8_t)hallActive,
                       (uint8_t)beltshift};

  _hal->packetSerial->send(message, sizeof(message));
  _apiTxTrafficIndication();
}

void API::_apiConfirmStart(bool success) {
  uint8_t message[] = {(uint8_t)AYAB_API::confirmStart, (uint8_t)success};
  _hal->packetSerial->send(message, sizeof(message));
  _apiTxTrafficIndication();
}

void API::_apiRequestLine(uint8_t rowNumber) {
  uint8_t message[] = {(uint8_t)AYAB_API::requestLine, (uint8_t)rowNumber};
  _hal->packetSerial->send(message, sizeof(message));
  _apiTxTrafficIndication();
}

void API::_apiConfirmInfo() {
  uint8_t message[] = {(uint8_t)AYAB_API::confirmInfo, API_VERSION,
                       FW_VERSION_MAJ, FW_VERSION_MIN};

  _hal->packetSerial->send(message, sizeof(message));
  _apiTxTrafficIndication();
}

void API::_apiConfirmPeek(uint8_t returnValue) {
  uint8_t message[] = {(uint8_t)AYAB_API::debugConfirmPeek, returnValue};

  _hal->packetSerial->send(message, sizeof(message));
  _apiTxTrafficIndication();  
}