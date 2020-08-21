/*!
 * \file tester.h
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

#ifndef TESTER_H_
#define TESTER_H_

#include <Arduino.h>
//#include <SerialCommand.h>

#include "beeper.h"

constexpr uint8_t BUFFER_LEN = 20;

class TesterInterface {
public:
  virtual ~TesterInterface(){};

  // any methods that need to be mocked should go here
  virtual void setUp() = 0;
  virtual void loop() = 0;
  virtual bool getQuitFlag() = 0;
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
#ifndef AYAB_TESTS
  virtual void encoderAChange();
#endif
};

// Container class for the static methods that implement the hardware test
// commands. Originally these methods were called back by `SerialCommand`.
// Dependency injection is enabled using a pointer to a global instance of
// either `Tester` or `TesterMock`, both of which classes implement the
// pure virtual methods of `TesterInterface`.

class GlobalTester final {
private:
  // singleton class so private constructor is appropriate
  GlobalTester() = default;

public:
  // pointer to global instance whose methods are implemented
  static TesterInterface *m_instance;

  static void setUp();
  static void loop();
  static bool getQuitFlag();
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
#ifndef AYAB_TESTS
  static void encoderAChange();
#endif
};

class Tester : public TesterInterface {
#if AYAB_TESTS
  FRIEND_TEST(TesterTest, test_stopCmd);
  FRIEND_TEST(TesterTest, test_quitCmd);
  FRIEND_TEST(TesterTest, test_setUp);
  FRIEND_TEST(TesterTest, test_loop_default);
  FRIEND_TEST(TesterTest, test_loop_null);
  FRIEND_TEST(TesterTest, test_loop_autoTestEven);
  FRIEND_TEST(TesterTest, test_loop_autoTestOdd);
  // FRIEND_TEST(TesterTest, test_scanHex);
  friend class TesterTest;
#endif

public:
  void setUp();
  void loop();
  bool getQuitFlag();
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

  // static bool scanHex(char *str, uint8_t maxDigits, uint16_t *result);

  // SerialCommand m_sCmd = SerialCommand();

  bool m_quit = false;
  bool m_autoReadOn = false;
  bool m_autoTestOn = false;
  unsigned long m_lastTime = 0U;
  bool m_timerEventOdd = false;

  char buf[BUFFER_LEN] = {0};
};

#endif // TESTER_H_
