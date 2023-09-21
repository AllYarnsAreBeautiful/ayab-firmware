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

#ifndef FSM_H_
#define FSM_H_

#include <Arduino.h>

#include "encoders.h"
#include "op.h"

class FsmInterface {
public:
  virtual ~FsmInterface() = default;

  // any methods that need to be mocked should go here
  virtual void init() = 0;
  virtual void update() = 0;
  virtual void cacheEncoders() = 0;
  virtual void setState(OpInterface *state) = 0;
  virtual OpInterface *getState() = 0;
  virtual void setMachineType(Machine_t) = 0;
  virtual Machine_t getMachineType() = 0;
  virtual BeltShift_t getBeltShift() = 0;
  virtual Carriage_t getCarriage() = 0;
  virtual Direction_t getDirection() = 0;
  virtual Direction_t getHallActive() = 0;
  virtual uint8_t getPosition() = 0;
};

// Singleton container class for static methods.
// Dependency injection is enabled using a pointer
// to a global instance of either `Fsm` or `FsmMock`
// both of which classes implement the pure virtual methods
// of the `FsmInterface` class.

class GlobalFsm final {
private:
  // singleton class so private constructor is appropriate
  GlobalFsm() = default;

public:
  // pointer to global instance whose methods are implemented
  static FsmInterface *m_instance;

  static void init();
  static void update();
  static void cacheEncoders();
  static void setState(OpInterface *state);
  static OpInterface *getState();
  static void setMachineType(Machine_t);
  static Machine_t getMachineType();
  static BeltShift_t getBeltShift();
  static Carriage_t getCarriage();
  static Direction_t getDirection();
  static Direction_t getHallActive();
  static uint8_t getPosition();
};

class Fsm : public FsmInterface {
public:
  void init() final;
  void update() final;
  void cacheEncoders() final;
  void setState(OpInterface *state) final;
  OpInterface *getState() final;
  void setMachineType(Machine_t) final;
  Machine_t getMachineType() final;
  BeltShift_t getBeltShift() final;
  Carriage_t getCarriage() final;
  Direction_t getDirection() final;
  Direction_t getHallActive() final;
  uint8_t getPosition() final;

  // machine state
  OpInterface *m_currentState;
  OpInterface *m_nextState;

  // machine type
  Machine_t m_machineType;

  // cached Encoder values
  BeltShift_t m_beltShift;
  Carriage_t m_carriage;
  Direction_t m_direction;
  Direction_t m_hallActive;
  uint8_t m_position;

#if AYAB_TESTS
  // Note: ideally tests would only rely on the public interface.
  FRIEND_TEST(TestOpKnit, test_getStartOffset);
  FRIEND_TEST(TestFsm, test_update_init);
#endif
};

#endif // FSM_H_
