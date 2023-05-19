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

#ifndef FSM_H_
#define FSM_H_

enum OpState { s_wait_for_machine = 0, s_init = 1, s_ready = 2, s_knit = 3, s_test = 4, s_error = 5};
using OpState_t = enum OpState;

// As of APIv6, the only important distinction
// is between `SUCCESS` (0) and any other value.
// Informative error codes are provided for
// diagnostic purposes (that is, for debugging).
// Non-zero error codes are subject to change.
// Such changes will be considered non-breaking.
enum ErrorCode {
  SUCCESS = 0x00,

  // message not understood
  EXPECTED_LONGER_MESSAGE = 0x01,
  UNRECOGNIZED_MSGID = 0x02,
  UNEXPECTED_MSGID = 0x03,
  CHECKSUM_ERROR = 0x04,

  // invalid arguments
  MACHINE_TYPE_INVALID = 0x10,
  NEEDLE_VALUE_INVALID = 0x11,
  NULL_POINTER_ARGUMENT = 0x12,
  ARGUMENT_INVALID = 0x13,
  ARGUMENTS_INCOMPATIBLE = 0x13,

  // device not initialized
  NO_MACHINE_TYPE = 0x20,
  NO_CARRIAGE = 0x21,
  NO_DIRECTION = 0x22,
  NO_BELTSHIFT = 0x23,

  // machine in wrong FSM state
  MACHINE_STATE_INIT = 0xE0,
  MACHINE_STATE_READY = 0xE1,
  MACHINE_STATE_KNIT = 0xE2,
  MACHINE_STATE_TEST = 0xE3,
  WRONG_MACHINE_STATE = 0xEF,

  // generic error codes
  WARNING = 0xF0, // ignorable error
  RECOVERABLE_ERROR = 0xF1,
  CRITICAL_ERROR = 0xF2,
  FATAL_ERROR = 0xF3,
  UNSPECIFIED_FAILURE = 0xFF
};
using Err_t = enum ErrorCode;

class FsmInterface {
public:
  virtual ~FsmInterface(){};

  // any methods that need to be mocked should go here
  virtual void init() = 0;
  virtual OpState_t getState() = 0;
  virtual void setState(OpState_t state) = 0;
  virtual void dispatch() = 0;
};

// Singleton container class for static methods.
// Dependency injection is enabled using a pointer
// to a global instance of either `Knitter` or `KnitterMock`
// both of which classes implement the pure virtual methods
// of the `KnitterInterface` class.

class GlobalFsm final {
private:
  // singleton class so private constructor is appropriate
  GlobalFsm() = default;

public:
  // pointer to global instance whose methods are implemented
  static FsmInterface *m_instance;

  static void init();
  static OpState_t getState();
  static void setState(OpState_t state);
  static void dispatch();
};

class Fsm : public FsmInterface {
public:
  void init();
  OpState_t getState();
  void setState(OpState_t state);
  void dispatch();

private:
  void state_init();
  void state_ready();
  void state_knit();
  void state_test();
  void state_error();

  // machine state
  OpState_t m_currentState;
  OpState_t m_nextState;

  // error state
  Err_t m_error;

  // flashing LEDs in error state
  bool m_flash;
  unsigned long m_flashTime;
};

#endif // FSM_H_
