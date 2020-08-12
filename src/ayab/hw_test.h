/*!
 * \file hw_test.h
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

#ifndef HW_TEST_H_
#define HW_TEST_H_

#include <Arduino.h>
#include <SerialCommand.h>

#include "beeper.h"

class HardwareTest {
#if AYAB_TESTS
  FRIEND_TEST(HardwareTestTest, test_setUp);
  FRIEND_TEST(HardwareTestTest, test_loop_default);
  FRIEND_TEST(HardwareTestTest, test_loop_autoRead);
  FRIEND_TEST(HardwareTestTest, test_loop_autoTestEven);
  FRIEND_TEST(HardwareTestTest, test_loop_autoTestOdd);
  friend class HardwareTestTest;
#endif
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

  static SerialCommand m_sCmd;
  /* static bool m_quitFlag; */

private:
  static void beep();
  static void readEOLsensors();
  static void readEncoders();
  static void autoRead();
  static void autoTestEven();
  static void autoTestOdd();
  static void handleTimerEvent();

#ifndef AYAB_TESTS
  static void encoderAChange();
#endif

  static bool m_autoReadOn;
  static bool m_autoTestOn;

  static unsigned long m_lastTime;
  static bool m_timerEventOdd;
};

extern HardwareTest *hwTest;

#endif // HW_TEST_H_
