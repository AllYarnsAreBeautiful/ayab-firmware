/*!
 * \file opReady.h
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

#ifndef OP_READY_H_
#define OP_READY_H_

#include "op.h"

class OpReadyInterface : public OpInterface {
public:
  virtual ~OpReadyInterface() = default;
};

// Container class for the static methods that implement the hardware test
// commands. Dependency injection is enabled using a pointer to a global
// instance of either `OpReady` or `OpReadyMock`, both of which classes
// implement the pure virtual methods of the `OpReadyInterface` class.

class GlobalOpReady final {
private:
  // singleton class so private constructor is appropriate
  GlobalOpReady() = default;

public:
  // pointer to global instance whose methods are implemented
  static OpReadyInterface *m_instance;

  static OpState_t state();
  static void init();
  static void begin();
  static void update();
  static void com(const uint8_t *buffer, size_t size);
  static void end();
};

class OpReady : public OpReadyInterface {
public:
  OpState_t state() final;
  void init() final;
  void begin() final;
  void update() final;
  void com(const uint8_t *buffer, size_t size) final;
  void end() final;
};

#endif // OP_READY_H_
