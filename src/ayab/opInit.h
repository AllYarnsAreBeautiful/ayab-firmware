/*!
 * \file opInit.h
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

#ifndef OP_INIT_H_
#define OP_INIT_H_

#include "op.h"
#include "encoders.h"

class OpInitInterface : public OpInterface {
public:
  virtual ~OpInitInterface() = default;

  virtual bool isReady() = 0;
};

// Container class for the static methods that implement the hardware test
// commands. Dependency injection is enabled using a reference to a global
// instance of either `OpInit` or `OpInitMock`, both of which classes
// implement the pure virtual methods of the `OpInitInterface` class.

class GlobalOpInit final {
private:
  // singleton class so private constructor is appropriate
  GlobalOpInit() = default;

public:
  // reference to global instance whose methods are implemented
  static OpInitInterface& m_instance;

  static OpState_t state();
  static void init();
  static void begin();
  static void update();
  static void com(const uint8_t *buffer, size_t size);
  static void end();

  static bool isReady();
};

class OpInit : public OpInitInterface {
public:
  OpState_t state() final;
  void init() final;
  void begin() final;
  void update() final;
  void com(const uint8_t *buffer, size_t size) final;
  void end() final;

  bool isReady() final;

private:
  Direction_t m_lastHall;
#ifdef DBG_NOMACHINE
  bool m_prevState;
#endif

#if AYAB_TESTS
  // Note: ideally tests would only rely on the public interface.
  FRIEND_TEST(OpInitTest, test_init);
#endif
};

#endif // OP_INIT_H_
