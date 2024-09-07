#ifndef API_H
#define API_H

#include <stdint.h>

#include "hal.h"

#define API_VERSION 5

enum class MachineType : uint8_t {
  NoMachine = 0xff,
  Kh910 = 0,
  Kh930 = 1,
  Kh270 = 2
};

enum class CarriageType : uint8_t { NoCarriage = 0, Knit = 1, Lace = 2, Gartner = 3 };

enum class BeltShift : uint8_t { Unknown = 0, Regular = 1, Shifted = 2 };

enum class Direction : uint8_t { Unknown = 0, Left = 1, Right = 2 };

enum class AYAB_API : uint8_t {
  requestReset = 0x00,
  requestStart = 0x01,
  confirmStart = 0xc1,
  requestLine = 0x82,
  confirmLine = 0x42,
  requestInfo = 0x03,
  confirmInfo = 0xc3,
  requestState = 0x44,
  indicateState = 0x84,

  debugBase = 0xf0,
  debugRequestPeek = 0xf8,
  debugConfirmPeek = 0xf9,
  debugRequestPoke = 0xfa
};

class API {
 public:
  API(hardwareAbstraction::HalInterface *hal);
  virtual ~API() = default;
  // Rx handler  for PacketSerial
  void rxMessageHandler(const uint8_t *buffer, size_t size);
  // Update object's state and run callbacks
  void schedule();

 protected:
  // Call derived class method indicating Rx traffic
  void virtual _apiRxTrafficIndication() = 0;
  // Call derived class method indicating Tx traffic
  void virtual _apiTxTrafficIndication() = 0;

  //----------------------------------------------------------------------------
  // RX message handlers
  //----------------------------------------------------------------------------

  // Call derived class method to reset the machine
  void virtual _apiRequestReset() = 0;
  // Call derived class method to set the machine's configuration
  bool virtual _apiRxSetConfig(uint8_t startNeedle, uint8_t stopNeedle,
                               bool continuousReporting) = 0;
  // Call derived class method to set the line pattern
  bool virtual _apiRxSetLine(uint8_t lineNumber, const uint8_t *pattern,
                             bool isLastLine) = 0;
  // Call derived class method to report knitter state
  void virtual _apiRxIndicateState() = 0;

  //----------------------------------------------------------------------------
  // TX message handlers
  //----------------------------------------------------------------------------

  // Send message to indicate machine state
  void _apiIndicateState(uint16_t hallValueLeft, uint16_t hallValueRight,
                         bool initDone, CarriageType carriageType,
                         uint8_t carriagePosition, Direction carriageDirection,
                         Direction hallActive, BeltShift beltshift);
  // Reply to the start request message
  void _apiConfirmStart(bool success);
  // Request for a new line pattern
  void _apiRequestLine(uint8_t rowNumber);
  // Reply to the info request message
  void _apiConfirmInfo();
  // Return peek value
  void _apiConfirmPeek(uint8_t returnValue);

  hardwareAbstraction::HalInterface *_hal;
};

#endif