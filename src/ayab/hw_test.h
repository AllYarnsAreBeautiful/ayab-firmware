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

class HardwareTestInterface {
public:
  virtual ~HardwareTestInterface(){};

  virtual void helpCmd() = 0;
  virtual void sendCmd() = 0;
  virtual void beepCmd() = 0;
  virtual void setSingleCmd() = 0;
  virtual void setAllCmd() = 0;
  virtual void readEOLsensorsCmd() = 0;
  virtual void readEncodersCmd() = 0;
  virtual void autoReadCmd() = 0;
  virtual void autoTestCmd() = 0;
  virtual void stopCmd() = 0;
  virtual void quitCmd() = 0;
  virtual void unrecognizedCmd(const char *buffer) = 0;

  virtual void setUp() = 0;
  virtual void loop() = 0;
#ifndef AYAB_TESTS
  virtual void encoderAChange() = 0;
#endif
};

class HardwareTest : public HardwareTestInterface {
#if AYAB_TESTS
  FRIEND_TEST(HardwareTestTest, test_setUp);
  FRIEND_TEST(HardwareTestTest, test_loop_default);
  FRIEND_TEST(HardwareTestTest, test_loop_null);
  FRIEND_TEST(HardwareTestTest, test_loop_autoTestEven);
  FRIEND_TEST(HardwareTestTest, test_loop_autoTestOdd);
  friend class HardwareTestTest;
#endif

public:
  void helpCmd();
  void sendCmd();
  void beepCmd();
  void setSingleCmd();
  void setAllCmd();
  void readEOLsensorsCmd();
  void readEncodersCmd();
  void autoReadCmd();
  void autoTestCmd();
  void stopCmd();
  void quitCmd();
  void unrecognizedCmd(const char *buffer);

  void setUp();
  void loop();
#ifndef AYAB_TESTS
  void encoderAChange();
#endif

private:
  void beep();
  void readEOLsensors();
  void readEncoders();
  void autoRead();
  void autoTestEven();
  void autoTestOdd();
  void handleTimerEvent();

  SerialCommand m_sCmd = SerialCommand();

  bool m_autoReadOn = false;
  bool m_autoTestOn = false;

  unsigned long m_lastTime = 0U;
  bool m_timerEventOdd = false;
};

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

  static HardwareTestInterface *m_instance;
};

extern GlobalHardwareTest *hwTest;

#endif // HW_TEST_H_
