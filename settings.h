// settings.h
/*
This file is part of AYAB.

    AYAB is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    AYAB is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with AYAB.  If not, see <http://www.gnu.org/licenses/>.

    Copyright 2013 Christian Obersteiner, Andreas Müller
    https://bitbucket.org/chris007de/ayab-apparat/
*/

#ifndef SETTINGS_H
#define SETTINGS_H

#define API_VERSION 3

#define SERIAL_BAUDRATE 115200

#define BEEPDELAY 50 // ms

#define EOL_PIN_R 0	// Analog
#define EOL_PIN_L 1	// Analog

#define ENC_PIN_A 2
#define ENC_PIN_B 3
#define ENC_PIN_C 4

#define LED_PIN_A 5 // green LED
#define LED_PIN_B 6 // yellow LED

#define PIEZO_PIN 9

// Should be calibrated to each device
#define FILTER_L_MIN 0
#define FILTER_L_MAX 600
#define FILTER_R_MIN 200
#define FILTER_R_MAX 1023
//
// Machine constants
#define NUM_NEEDLES    200
#define END_LEFT       0
#define END_RIGHT      255
#define START_OFFSET_L 40
#define START_OFFSET_R 16
#define END_OF_LINE_OFFSET_L 32
#define END_OF_LINE_OFFSET_R 12

// Dont touch
#define uint16 unsigned int

typedef enum Direction{
	NoDirection	= 0,
	Left  		= 1,
	Right 		= 2
} Direction_t;

typedef enum Beltshift{
	Unknown     = 0,
	Regular 	= 1, 
	Shifted   	= 2
} Beltshift_t;

typedef enum OpState{
	s_init      = 0,
	s_ready     = 1,
	s_operate   = 2
} OpState_t;
//


#endif