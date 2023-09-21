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
 *    Modified Work Copyright 2020-3 Sturla Lange, Tom Price
 *    http://ayab-knitting.com
 */

#ifndef OP_H_
#define OP_H_

#include <Arduino.h>

enum class OpState {
  Idle = 0,
  Init = 1,
  Ready = 2,
  Knit = 3,
  Test = 4,
  Error = 5};
using OpState_t = enum OpState;

// As of APIv6, the only important distinction
// is between `Err_t::Success` (0) and any other value.
// Informative error codes are provided for
// diagnostic purposes (that is, for debugging).
// Non-zero error codes are subject to change.
// Such changes will be considered non-breaking.
enum class ErrorCode : unsigned char {
  Success = 0x00,

  // message not understood
  Expected_longer_message = 0x01,
  Unrecognized_msgid = 0x02,
  Unexpected_msgid = 0x03,
  Checksum_error = 0x04,

  // invalid arguments
  Machine_type_invalid = 0x10,
  Needle_value_invalid = 0x11,
  Null_pointer_argument = 0x12,
  Argument_invalid = 0x13,
  Arguments_incompatible = 0x13,

  // device not initialized
  No_machine_type = 0x20,
  No_carriage = 0x21,
  No_direction = 0x22,
  No_beltshift = 0x23,

  // machine in wrong FSM state
  Machine_state_init = 0xE0,
  Machine_state_ready = 0xE1,
  Machine_state_knit = 0xE2,
  Machine_state_test = 0xE3,
  Wrong_machine_state = 0xEF,

  // generic error codes
  Warning = 0xF0, // ignorable error
  Recoverable_error = 0xF1,
  Critical_error = 0xF2,
  Fatal_error = 0xF3,
  Unspecified_failure = 0xFF
};
using Err_t = enum ErrorCode;

class OpInterface {
public:
  virtual ~OpInterface() = default;

  // any methods that need to be mocked should go here
  virtual OpState_t state() = 0;
  virtual void init() = 0;
  virtual void begin() = 0;
  virtual void update() = 0;
  virtual void com(const uint8_t *buffer, size_t size) = 0;
  virtual void end() = 0;
};

#endif // OP_H_
