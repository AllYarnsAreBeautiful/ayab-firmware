/*!`
 * \file controller.h
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

#ifndef CONTROLLER_H_
#define CONTROLLER_H_

#include <Arduino.h>

#include "encoders.h"
#include "op.h"

class ControllerInterface {
public:
  virtual ~ControllerInterface() = default;

  // any methods that need to be mocked should go here
  virtual void init() = 0;
  virtual void update() = 0;
  virtual void com(const uint8_t *buffer, size_t size) const = 0;
  virtual void cacheEncoders() = 0;
  virtual void setState(OpInterface *state) = 0;
  virtual OpInterface *getState() const = 0;
  virtual void setMachineType(Machine_t) = 0;
  virtual Machine_t getMachineType() const = 0;
  virtual BeltShift_t getBeltShift() const = 0;
  virtual Carriage_t getCarriage() const = 0;
  virtual Direction_t getDirection() const = 0;
  virtual Direction_t getHallActive() const = 0;
  virtual uint8_t getPosition() const = 0;
};

// Singleton container class for static methods.
// Dependency injection is enabled using a pointer
// to a global instance of either `Controller` or `ControllerMock`
// both of which classes implement the pure virtual methods
// of the `ControllerInterface` class.

class GlobalController final {
private:
  // singleton class so private constructor is appropriate
  GlobalController() = default;

public:
  // pointer to global instance whose methods are implemented
  static ControllerInterface *m_instance;

  static void init();
  static void update();
  static void com(const uint8_t *buffer, size_t size);
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

class Controller : public ControllerInterface {
public:
  void init() final;
  void update() final;
  void com(const uint8_t *buffer, size_t size) const final;
  void cacheEncoders() final;
  void setState(OpInterface *state) final;
  OpInterface *getState() const final;
  void setMachineType(Machine_t) final;
  Machine_t getMachineType() const final;
  BeltShift_t getBeltShift() const final;
  Carriage_t getCarriage() const final;
  Direction_t getDirection() const final;
  Direction_t getHallActive() const final;
  uint8_t getPosition() const final;

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
  FRIEND_TEST(TestController, test_update_init);
#endif
};

#endif // CONTROLLER_H_
