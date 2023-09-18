/*!
 * \file init.h
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

#ifndef INIT_H_
#define INIT_H_

#include "op.h"

class InitInterface : public OpInterface {
public:
  virtual ~InitInterface() = default;
};

// Container class for the static methods that implement the hardware test
// commands. Dependency injection is enabled using a pointer to a global
// instance of either `Init` or `InitMock`, both of which classes
// implement the pure virtual methods of the `InitInterface` class.

class GlobalInit final {
private:
  // singleton class so private constructor is appropriate
  GlobalInit() = default;

public:
  // pointer to global instance whose methods are implemented
  static InitInterface *m_instance;

  static void init();
  static Err_t begin();
  static void update();
  static void com();
  static void end();
};

class Init : public InitInterface {
public:
  void init() final;
  Err_t begin() final;
  void update() final;
  void com() final;
  void end() final;
};

#endif // INIT_H_
