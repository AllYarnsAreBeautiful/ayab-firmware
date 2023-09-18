/*!
 * \file test.h
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

#ifndef TEST_H_
#define TEST_H_

#include <Arduino.h>

#include "beeper.h"
#include "com.h"
#include "encoders.h"
#include "op.h"

constexpr uint8_t BUFFER_LEN = 40;
constexpr uint16_t TEST_LOOP_DELAY = 500; // ms

class TestInterface : public OpInterface {
public:
  virtual ~TestInterface() = default;

  // any methods that need to be mocked should go here
  virtual bool enabled() = 0;
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
#ifndef AYAB_TESTS
  virtual void encoderAChange();
#endif
};

// Container class for the static methods that implement the hardware test
// commands. Dependency injection is enabled using a pointer to a global
// instance of either `Test` or `TestMock`, both of which classes
// implement the pure virtual methods of the `TestInterface` class.

class GlobalTest final {
private:
  // singleton class so private constructor is appropriate
  GlobalTest() = default;

public:
  // pointer to global instance whose methods are implemented
  static TestInterface *m_instance;

  static void init();
  static Err_t begin();
  static void update();
  static void com();
  static void end();

  static bool enabled();
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
#ifndef AYAB_TESTS
  static void encoderAChange();
#endif
};

class Test : public TestInterface {
public:
  void init() final;
  Err_t begin() final;
  void update() final;
  void com() final;
  void end() final;

  bool enabled() final;
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
#ifndef AYAB_TESTS
  void encoderAChange() final;
#endif

private:
  void beep() const;
  void readEOLsensors();
  void readEncoders() const;
  void autoRead();
  void autoTestEven() const;
  void autoTestOdd() const;
  void handleTimerEvent();

  bool m_autoReadOn = false;
  bool m_autoTestOn = false;
  uint32_t m_lastTime = 0U;
  bool m_timerEventOdd = false;

  char buf[BUFFER_LEN] = {0};
};

#endif // TEST_H_
