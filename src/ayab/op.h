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
// is between `ErrorCode::success` (0) and any other value.
// Informative error codes are provided for
// diagnostic purposes (that is, for debugging).
// Non-zero error codes are subject to change.
// Such changes will be considered non-breaking.
enum class ErrorCode : unsigned char {
  success = 0x00,

  // message not understood
  expected_longer_message = 0x01,
  unrecognized_msgid = 0x02,
  unexpected_msgid = 0x03,
  checksum_error = 0x04,

  // invalid arguments
  machine_type_invalid = 0x10,
  needle_value_invalid = 0x11,
  null_pointer_argument = 0x12,
  argument_invalid = 0x13,
  arguments_incompatible = 0x13,

  // device not initialized
  no_machine_type = 0x20,
  no_carriage = 0x21,
  no_direction = 0x22,
  no_beltshift = 0x23,

  // machine in wrong FSM state
  machine_state_init = 0xE0,
  machine_state_ready = 0xE1,
  machine_state_knit = 0xE2,
  machine_state_test = 0xE3,
  wrong_machine_state = 0xEF,

  // generic error codes
  warning = 0xF0, // ignorable error
  recoverable_error = 0xF1,
  critical_error = 0xF2,
  fatal_error = 0xF3,
  unspecified_failure = 0xFF
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
