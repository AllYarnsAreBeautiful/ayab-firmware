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

#include "beeper.h"
#include "com.h"
#include "encoders.h"

constexpr uint8_t BUFFER_LEN = 40;

class TesterInterface {
public:
  virtual ~TesterInterface(){};

  // any methods that need to be mocked should go here
  virtual Err_t startTest(Machine_t machineType) = 0;
  virtual void loop() = 0;
  virtual void helpCmd() = 0;
  virtual void sendCmd() = 0;
  virtual void beepCmd() = 0;
  virtual void setSingleCmd(const uint8_t *buffer, size_t size) = 0;
  virtual void setAllCmd(const uint8_t *buffer, size_t size) = 0;
  virtual void readEOLsensorsCmd() = 0;
  virtual void readEncodersCmd() = 0;
  virtual void autoReadCmd() = 0;
  virtual void autoTestCmd() = 0;
  virtual void stopCmd() = 0;
  virtual void quitCmd() = 0;
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

  static Err_t startTest(Machine_t machineType);
  static void loop();
  static void helpCmd();
  static void sendCmd();
  static void beepCmd();
  static void setSingleCmd(const uint8_t *buffer, size_t size);
  static void setAllCmd(const uint8_t *buffer, size_t size);
  static void readEOLsensorsCmd();
  static void readEncodersCmd();
  static void autoReadCmd();
  static void autoTestCmd();
  static void stopCmd();
  static void quitCmd();
#ifndef AYAB_TESTS
  static void encoderAChange();
#endif
};

class Tester : public TesterInterface {
public:
  Err_t startTest(Machine_t machineType);
  void loop();
  void helpCmd();
  void sendCmd();
  void beepCmd();
  void setSingleCmd(const uint8_t *buffer, size_t size);
  void setAllCmd(const uint8_t *buffer, size_t size);
  void readEOLsensorsCmd();
  void readEncodersCmd();
  void autoReadCmd();
  void autoTestCmd();
  void stopCmd();
  void quitCmd();
#ifndef AYAB_TESTS
  void encoderAChange();
#endif

private:
  void setUp();
  void beep();
  void readEOLsensors();
  void readEncoders();
  void autoRead();
  void autoTestEven();
  void autoTestOdd();
  void handleTimerEvent();

  bool m_autoReadOn = false;
  bool m_autoTestOn = false;
  unsigned long m_lastTime = 0U;
  bool m_timerEventOdd = false;

  char buf[BUFFER_LEN] = {0};
};

#endif // TESTER_H_
