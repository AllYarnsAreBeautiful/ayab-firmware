/*!
 * \file error.h
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

#ifndef ERROR_H_
#define ERROR_H_

#include <Arduino.h>

#include "op.h"

class ErrorInterface : public OpInterface {
public:
  virtual ~ErrorInterface() = default;
};

// Container class for the static methods that implement the hardware test
// commands. Dependency injection is enabled using a pointer to a global
// instance of either `Error` or `ErrorMock`, both of which classes
// implement the pure virtual methods of the `ErrorInterface` class.

class GlobalError final {
private:
  // singleton class so private constructor is appropriate
  GlobalError() = default;

public:
  // pointer to global instance whose methods are implemented
  static ErrorInterface *m_instance;

  static void init();
  static Err_t begin();
  static void update();
  static void com();
  static void end();
};

class Error : public ErrorInterface {
public:
  void init() final;
  Err_t begin() final;
  void update() final;
  void com() final;
  void end() final;

private:
  // error state
  Err_t m_error;

  // flashing LEDs in error state
  bool m_flash;
  uint32_t m_flashTime;
};

#endif // ERROR_H_
