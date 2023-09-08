/*!
 * \file solenoids.h
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

#ifndef SOLENOIDS_H_
#define SOLENOIDS_H_

#include "board.h"
#include "encoders.h"
#include <Arduino.h>
#include <Adafruit_MCP23008.h>
#include <Wire.h>

// Different machines have a different number of solenoids.
//                                              {910, 930, 270}
constexpr uint8_t SOLENOIDS_NUM[NUM_MACHINES] = {16U, 16U, 12U};
constexpr uint8_t HALF_SOLENOIDS_NUM[NUM_MACHINES] = {8U, 8U, 6U};
constexpr uint8_t SOLENOIDS_I2C_ADDRESS_MASK = 0x20U;
constexpr uint8_t SOLENOID_BUFFER_SIZE = 16U;

class SolenoidsInterface {
public:
  virtual ~SolenoidsInterface() = default;

  // any methods that need to be mocked should go here
  virtual void init() = 0;
  virtual void setSolenoid(uint8_t solenoid, bool state) = 0;
  virtual void setSolenoids(uint16_t state) = 0;
};

// Container class for the static methods that control the solenoids.
// Dependency injection is enabled using a pointer to a global instance of
// either `Solenoids` or `SolenoidsMock`, both of which classes implement
// the pure virtual methods of `SolenoidsInterface`.

class GlobalSolenoids final {
private:
  // singleton class so private constructor is appropriate
  GlobalSolenoids() = default;

public:
  // pointer to global instance whose methods are implemented
  static SolenoidsInterface *m_instance;

  static void init();
  static void setSolenoid(uint8_t solenoid, bool state);
  static void setSolenoids(uint16_t state);
};

class Solenoids : public SolenoidsInterface {
#ifdef AYAB_TESTS
  FRIEND_TEST(SolenoidsTest, test_init);
  FRIEND_TEST(SolenoidsTest, test_setSolenoid1);
  FRIEND_TEST(SolenoidsTest, test_setSolenoid2);
  FRIEND_TEST(SolenoidsTest, test_setSolenoid3);
#endif
public:
  Solenoids() = default;

  void init() final;
  void setSolenoid(uint8_t solenoid, bool state) final;
  void setSolenoids(uint16_t state) final;

private:
  uint16_t solenoidState = 0x0000U;
  void write(uint16_t state);

  Adafruit_MCP23008 mcp_0 = Adafruit_MCP23008();
  Adafruit_MCP23008 mcp_1 = Adafruit_MCP23008();
};

#endif // SOLENOIDS_H_
