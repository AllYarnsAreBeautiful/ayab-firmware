/*!
 * \file board.h
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
 *    Original Work Copyright 2013-2015 Christian Obersteiner, Andreas Müller
 *    Modified Work Copyright 2020 Sturla Lange, Tom Price
 *    http://ayab-knitting.com
 */

#ifndef BOARD_H_
#define BOARD_H_

#include <Arduino.h>

// Pin Assignments
constexpr uint8_t EOL_PIN_R = A0; // Analog
constexpr uint8_t EOL_PIN_L = A1; // Analog

constexpr uint8_t ENC_PIN_A = 2;
constexpr uint8_t ENC_PIN_B = 3;

constexpr uint8_t ENC_PIN_C = 4;

constexpr uint8_t LED_PIN_A = 5; // green LED
constexpr uint8_t LED_PIN_B = 6; // yellow LED

constexpr uint8_t PIEZO_PIN = 9;

#ifdef DBG_NOMACHINE               // Turn on to use DBG_BTN as EOL Trigger
constexpr uint8_t DBG_BTN_PIN = 7; // DEBUG BUTTON
#endif

constexpr uint8_t I2Caddr_sol1_8 = 0x0U;  ///< I2C Address of solenoids 1 - 8
constexpr uint8_t I2Caddr_sol9_16 = 0x1U; ///< I2C Address of solenoids 9 - 16

// TODO(Who?): Optimize Delay for various Arduino Models
constexpr uint16_t START_KNITTING_DELAY = 2000U;

// Determine board type
#if defined(__AVR_ATmega168__) || defined(__AVR_ATmega328P__)
// Arduino Uno
#define HARD_I2C

#elif defined(__AVR_ATmega2560__) || defined(__AVR_ATmega1280__)
// Arduino Mega
#define SOFT_I2C

#else
#error "untested board - please check your I2C ports"
#endif

#endif // BOARD_H_
