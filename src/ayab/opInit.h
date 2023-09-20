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
 *    Modified Work Copyright 2020 Sturla Lange, Tom Price
 *    http://ayab-knitting.com
 */

#ifndef OP_INIT_H_
#define OP_INIT_H_

#include "op.h"

class OpInitInterface : public OpInterface {
public:
  virtual ~OpInitInterface() = default;
};

// Container class for the static methods that implement the hardware test
// commands. Dependency injection is enabled using a pointer to a global
// instance of either `OpInit` or `OpInitMock`, both of which classes
// implement the pure virtual methods of the `OpInitInterface` class.

class GlobalOpInit final {
private:
  // singleton class so private constructor is appropriate
  GlobalOpInit() = default;

public:
  // pointer to global instance whose methods are implemented
  static OpInitInterface *m_instance;

  static void init();
  static Err_t begin();
  static void update();
  static void com(const uint8_t *buffer, size_t size);
  static void end();
};

class OpInit : public OpInitInterface {
public:
  void init() final;
  Err_t begin() final;
  void update() final;
  void com(const uint8_t *buffer, size_t size) final;
  void end() final;
};

#endif // OP_INIT_H_