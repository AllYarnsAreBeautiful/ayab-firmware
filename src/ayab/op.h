/*!`
 * \file op.h
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

#include "com.h"

class OpInterface {
public:
  virtual ~OpInterface() = default;

  // any methods that need to be mocked should go here
  virtual void init() = 0;
  virtual Err_t begin() = 0;
  virtual void update() = 0;
  virtual void com() = 0;
  virtual void end() = 0;
};

/*
// Singleton container class for static methods.
// Dependency injection is enabled using a pointer
// to a global instance of either `Op` or `OpMock`
// both of which classes implement the pure virtual methods
// of the `OpInterface` class.

class GlobalOp {
private:
  // singleton class so private constructor is appropriate
  GlobalOp() = default;

public:
  // pointer to global instance whose methods are implemented
  static OpInterface *m_instance;

  static void init();
  static Err_t begin();
  static void end();
  static void update();
  static void com();
};

class Op : public OpInterface {
public:
  void init() override;
  Err_t begin() override;
  void end() override;
  void update() override;
  void com() override;

private:
};
*/
#endif // OP_H_
