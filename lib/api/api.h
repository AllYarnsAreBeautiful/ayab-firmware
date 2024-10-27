#ifndef API_H
#define API_H

#include <stdint.h>

#include "hal.h"
#include "version.h"

#define API_VERSION 6

enum class ErrorCode : uint8_t {
  Success = 0x00,
  MessageIncorrectLenght = 0x01,
  MessageUnknown = 0x02,
  MessageChecksum = 0x04,
  MessageInvalidArguments = 0x13,
  MachineInvalidState = 0xef,
  Error = 0xff
};

enum class MachineType : uint8_t {
  NoMachine = 0xff,
  Kh910 = 0x00,
  Kh930 = 0X01,
  Kh270 = 0x02
};

enum class MachineSide { None = 0, Left = 1, Right = 2 };

enum class KnitterState : uint8_t {
  Reset = 0x00,
  Init = 0x01,
  Ready = 0x02,
  Operate = 0x03
};

enum class CarriageType : uint8_t {
  NoCarriage = 0xff,
  Knit = 0,
  Lace = 1,
  Gartner = 2,
  Knit270 = 3
};

enum class BeltShift : uint8_t { Unknown = 0, Regular = 1, Shifted = 2 };

enum class Direction : uint8_t { Unknown = 0xff, Left = 0, Right = 1 };

enum class AYAB_API : uint8_t {
  requestReset = 0x00,
  requestInit = 0x05,
  confirmInit = 0xc5,
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
  ErrorCode _error;

  // Call derived class method indicating Rx traffic
  void virtual _apiRxTrafficIndication() = 0;
  // Call derived class method indicating Tx traffic
  void virtual _apiTxTrafficIndication() = 0;

  //----------------------------------------------------------------------------
  // RX message handlers
  //----------------------------------------------------------------------------

  // Call derived class method to reset the machine
  void virtual _apiRequestReset() = 0;
  // Call derived class method to set the machine type
  ErrorCode virtual _apiRequestInit(MachineType machine) = 0;
  // Call derived class method to set the machine's configuration
  ErrorCode virtual _apiRxSetConfig(uint8_t startNeedle, uint8_t stopNeedle,
                                    bool continuousReporting,
                                    bool beeperEnabled) = 0;
  // Call derived class method to set the line pattern
  ErrorCode virtual _apiRxSetLine(uint8_t lineNumber, const uint8_t *pattern,
                                  uint8_t size, bool isLastLine) = 0;
  // Call derived class method to report knitter state
  void virtual _apiRxIndicateState() = 0;

  //----------------------------------------------------------------------------
  // TX message handlers
  //----------------------------------------------------------------------------
  // Send message to indicate machine state
  void _apiIndicateState(KnitterState state, uint16_t hallValueLeft,
                         uint16_t hallValueRight, CarriageType carriageType,
                         int16_t carriagePosition, Direction carriageDirection,
                         MachineSide hallActive, BeltShift beltshift);
  // Reply to init request message (set machne type)
  void _apiConfirmInit();
  // Reply to the start request message
  void _apiConfirmStart();
  // Request for a new line pattern
  void _apiRequestLine(uint8_t rowNumber, ErrorCode error);
  // Reply to the info request message
  void _apiConfirmInfo();
  // Return peek value
  void _apiConfirmPeek(uint8_t returnValue);

  hardwareAbstraction::HalInterface *_hal;
};

#endif