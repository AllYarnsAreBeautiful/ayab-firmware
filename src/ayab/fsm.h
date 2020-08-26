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

#include "com.h"

enum OpState { s_init, s_ready, s_knit, s_test, s_error };
using OpState_t = enum OpState;

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
