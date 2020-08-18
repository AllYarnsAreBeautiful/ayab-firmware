/*!
 * \file global_hw_test.h
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

#ifndef GLOBAL_HW_TEST_H_
#define GLOBAL_HW_TEST_H_

#include "hw_test.h"

// This is a container for the static methods called back by SerialCommand.
// Dependency injection is enabled using a pointer to a global instance of
// either HardwareTest or HardwareTestMock, both of which classes implement
// the pure virtual methods of HardwareTestInterface.

class GlobalHardwareTest {
public:
  static void helpCmd();
  static void sendCmd();
  static void beepCmd();
  static void setSingleCmd();
  static void setAllCmd();
  static void readEOLsensorsCmd();
  static void readEncodersCmd();
  static void autoReadCmd();
  static void autoTestCmd();
  static void stopCmd();
  static void quitCmd();
  static void unrecognizedCmd(const char *buffer);

  static void setUp();
  static void loop();
#ifndef AYAB_TESTS
  static void encoderAChange();
#endif

  // pointer to global instance whose methods are implemented
  // in the static methods belonging to this class
  static HardwareTestInterface *m_instance;
};

extern GlobalHardwareTest *hwTest;

#endif // GLOBAL_HW_TEST_H_
