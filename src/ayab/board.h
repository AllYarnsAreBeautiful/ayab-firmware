#pragma once
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
 *    Copyright 2013-2015 Christian Obersteiner, Andreas Müller
 *    http://ayab-knitting.com
 */

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
