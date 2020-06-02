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
 *    Copyright 2013 Christian Obersteiner, Andreas Müller
 *    http://ayab-knitting.com
 */

#ifndef SOLENOIDS_H_
#define SOLENOIDS_H_

#include <Arduino.h>

#include "settings.h"

#define I2Caddr_sol1_8 0x0  ///< I2C Address of solenoids 1 - 8
#define I2Caddr_sol9_16 0x1 ///< I2C Address of solenoids 9 - 16

// Determine board type
#if defined(__AVR_ATmega168__) || defined(__AVR_ATmega328P__)
// Arduino Uno
#if !defined(AYAB_QUIET)
#warning Using Hardware I2C
#endif
#ifndef HARD_I2C
#define HARD_I2C
#endif

#elif defined(__AVR_ATmega2560__) || defined(__AVR_ATmega1280__)
// Arduino Mega
#if !defined(AYAB_QUIET)
#warning Using Software I2C
#endif
#ifndef SOFT_I2C
#define SOFT_I2C
#endif
#else
#error "untested board - please check your I2C ports"
#endif

/*!
 * \brief Control of the needles via solenoids connected to IO expanders.
 */
class Solenoids {
public:
  Solenoids();
  void init(void);
  void setSolenoid(uint8_t solenoid, bool state);
  void setSolenoids(uint16_t state);

private:
  uint16_t solenoidState = 0x00;
  void write(uint16_t state);
};

#endif // SOLENOIDS_H_
