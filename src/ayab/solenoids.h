#pragma once
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
 *    Modified Work Copyright 2020 Sturla Lange
 *    http://ayab-knitting.com
 */

#include <Arduino.h>

#include "board.h"

#if defined(HARD_I2C)
#include <Adafruit_MCP23008.h>
#include <Wire.h>
#elif defined(SOFT_I2C)
#include <SoftI2CMaster.h>
#endif

constexpr uint8_t SOLENOIDS_NUM = 16U;
constexpr uint8_t SOLENOIDS_I2C_ADDRESS_MASK = 0x20U;

/*!
 * \brief Control of the needles via solenoids connected to IO expanders.
 */
class Solenoids {
public:
  Solenoids()
#if defined(HARD_I2C)
      : mcp_0(), mcp_1()
#elif defined(SOFT_I2C)
      : SoftI2C(A4, A5)
#endif
  {
  }

  void init();
  void setSolenoid(uint8_t solenoid, bool state);
  void setSolenoids(uint16_t state);

private:
  uint16_t solenoidState = 0x0000U;
  void write(uint16_t state);

#if defined(HARD_I2C)
  Adafruit_MCP23008 mcp_0;
  Adafruit_MCP23008 mcp_1;
#elif defined(SOFT_I2C)
  SoftI2CMaster SoftI2C;
#endif
};
