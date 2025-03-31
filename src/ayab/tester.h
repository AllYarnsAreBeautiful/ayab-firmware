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
constexpr unsigned int TEST_LOOP_DELAY = 500; // ms

class TesterInterface {
public:
  virtual ~TesterInterface() = default;

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
  virtual void encoderChange();
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
  static void encoderChange();
#endif
};

class Tester : public TesterInterface {
public:
  Err_t startTest(Machine_t machineType) final;
  void loop() final;
  void helpCmd() final;
  void sendCmd() final;
  void beepCmd() final;
  void setSingleCmd(const uint8_t *buffer, size_t size) final;
  void setAllCmd(const uint8_t *buffer, size_t size) final;
  void readEOLsensorsCmd() final;
  void readEncodersCmd() final;
  void autoReadCmd() final;
  void autoTestCmd() final;
  void stopCmd() final;
  void quitCmd() final;
#ifndef AYAB_TESTS
  void encoderChange() final;
#endif

private:
  void setUp();
  void beep() const;
  void readEOLsensors();
  void readEncoders() const;
  void autoRead();
  void autoTestEven() const;
  void autoTestOdd() const;
  void handleTimerEvent();

  Machine_t m_machineType = Machine_t::NoMachine;
  bool m_autoReadOn = false;
  bool m_autoTestOn = false;
  unsigned long m_lastTime = 0U;
  bool m_timerEventOdd = false;

  char buf[BUFFER_LEN] = {0};
};

#endif // TESTER_H_
