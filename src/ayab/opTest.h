/*!
 * \file opTest.h
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

#ifndef OP_TEST_H_
#define OP_TEST_H_

#include <Arduino.h>

#include "beeper.h"
#include "com.h"
#include "encoders.h"
#include "op.h"

constexpr uint8_t BUFFER_LEN = 40;
constexpr uint16_t TEST_LOOP_DELAY = 500; // ms

class OpTestInterface : public OpInterface {
public:
  virtual ~OpTestInterface() = default;

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
// commands. Dependency injection is enabled using a reference to a global
// instance of either `OpTest` or `OpTestMock`, both of which classes
// implement the pure virtual methods of the `OpTestInterface` class.

class GlobalOpTest final {
private:
  // singleton class so private constructor is appropriate
  GlobalOpTest() = default;

public:
  // reference to global instance whose methods are implemented
  static OpTestInterface& m_instance;

  static OpState_t state();
  static void init();
  static void begin();
  static void update();
  static void com(const uint8_t *buffer, size_t size);
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

class OpTest : public OpTestInterface {
public:
  OpState_t state() final;
  void init() final;
  void begin() final;
  void update() final;
  void com(const uint8_t *buffer, size_t size) final;
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

#endif // OP_TEST_H_
