/*!`
 * \file fsm.h
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

#ifndef OP_H_
#define OP_H_

#include "encoders.h"

enum class OpState : unsigned char {
  wait_for_machine,
  init,
  ready,
  knit,
  test,
  error
};
using OpState_t = enum OpState;

// As of APIv6, the only important distinction
// is between `SUCCESS` (0) and any other value.
// Informative error codes are provided for
// diagnostic purposes (that is, for debugging).
// Non-zero error codes are subject to change.
// Such changes will be considered non-breaking.
enum class ErrorCode : unsigned char {
  SUCCESS = 0x00,

  // message not understood
  ERR_EXPECTED_LONGER_MESSAGE = 0x01,
  ERR_UNRECOGNIZED_MSGID = 0x02,
  ERR_UNEXPECTED_MSGID = 0x03,
  ERR_CHECKSUM_ERROR = 0x04,

  // invalid arguments
  ERR_MACHINE_TYPE_INVALID = 0x10,
  ERR_NEEDLE_VALUE_INVALID = 0x11,
  ERR_NULL_POINTER_ARGUMENT = 0x12,
  ERR_ARGUMENT_INVALID = 0x13,
  ERR_ARGUMENTS_INCOMPATIBLE = 0x13,

  // device not initialized
  ERR_NO_MACHINE_TYPE = 0x20,
  ERR_NO_CARRIAGE = 0x21,
  ERR_NO_DIRECTION = 0x22,
  ERR_NO_BELTSHIFT = 0x23,

  // machine in wrong FSM state
  ERR_MACHINE_STATE_INIT = 0xE0,
  ERR_MACHINE_STATE_READY = 0xE1,
  ERR_MACHINE_STATE_KNIT = 0xE2,
  ERR_MACHINE_STATE_TEST = 0xE3,
  ERR_WRONG_MACHINE_STATE = 0xEF,

  // generic error codes
  WARNING = 0xF0, // ignorable error
  RECOVERABLE_ERROR = 0xF1,
  CRITICAL_ERROR = 0xF2,
  FATAL_ERROR = 0xF3,
  UNSPECIFIED_FAILURE = 0xFF
};
using Err_t = enum ErrorCode;

constexpr unsigned int FLASH_DELAY = 500; // ms

class OpInterface {
public:
  virtual ~OpInterface() = default;

  // any methods that need to be mocked should go here
  virtual void init() = 0;
  virtual void update() = 0;
  virtual void cacheEncoders() = 0;
  virtual void setState(OpState_t state) = 0;
  virtual OpState_t getState() = 0;
  virtual BeltShift_t getBeltShift() = 0;
  virtual Carriage_t getCarriage() = 0;
  virtual Direction_t getDirection() = 0;
  virtual Direction_t getHallActive() = 0;
  virtual uint8_t getPosition() = 0;
};

// Singleton container class for static methods.
// Dependency injection is enabled using a pointer
// to a global instance of either `Knitter` or `KnitterMock`
// both of which classes implement the pure virtual methods
// of the `KnitterInterface` class.

class GlobalOp final {
private:
  // singleton class so private constructor is appropriate
  GlobalOp() = default;

public:
  // pointer to global instance whose methods are implemented
  static OpInterface *m_instance;

  static void init();
  static void update();
  static void cacheEncoders();
  static void setState(OpState_t state);
  static OpState_t getState();
  static BeltShift_t getBeltShift();
  static Carriage_t getCarriage();
  static Direction_t getDirection();
  static Direction_t getHallActive();
  static uint8_t getPosition();
};

class Op : public OpInterface {
public:
  void init() final;
  void update() final;
  void cacheEncoders() final;
  void setState(OpState_t state) final;
  OpState_t getState() final;
  BeltShift_t getBeltShift() final;
  Carriage_t getCarriage() final;
  Direction_t getDirection() final;
  Direction_t getHallActive() final;
  uint8_t getPosition() final;

private:
  void state_wait_for_machine() const;
  void state_init();
  void state_ready() const;
  void state_knit() const;
  void state_test() const;
  void state_error();

  // machine state
  OpState_t m_currentState;
  OpState_t m_nextState;

  // error state
  Err_t m_error;

  // flashing LEDs in error state
  bool m_flash;
  unsigned long m_flashTime;

  // cached Encoder values
  BeltShift_t m_beltShift;
  Carriage_t m_carriage;
  Direction_t m_direction;
  Direction_t m_hallActive;
  uint8_t m_position;
};

#endif // OP_H_
