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
#pragma once

// Pin Assignments
#define EOL_PIN_R A0 // Analog
#define EOL_PIN_L A1 // Analog

#define ENC_PIN_A 2
#define ENC_PIN_B 3

#define ENC_PIN_C 4

#define LED_PIN_A 5 // green LED
#define LED_PIN_B 6 // yellow LED

#define PIEZO_PIN 9

#ifdef DBG_NOMACHINE  // Turn on to use DBG_BTN as EOL Trigger
#define DBG_BTN_PIN 7 // DEBUG BUTTON
#endif
