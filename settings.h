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

    Copyright 2013-2015 Christian Obersteiner, Andreas Müller
    https://bitbucket.org/chris007de/ayab-apparat/
*/

#ifndef SETTINGS_H
#define SETTINGS_H

/*
 * USERSETTINGS
 */

//#define DBG_NOMACHINE  // Turn on to use DBG_BTN as EOL Trigger

#ifdef KH910
	#warning USING MACHINETYPE KH910
#else
	#ifdef KH930
		#warning USING MACHINETYPE KH930
	#else
		#error KH910 or KH930 has the be defined as preprocessor variable!
	#endif
#endif

// Should be calibrated to each device
// These values are for the K carriage
#ifdef KH910
    #define FILTER_L_MIN 200 // below: L Carriage
    #define FILTER_L_MAX 600 // above: K Carriage
    #define FILTER_R_MIN 200
    #define FILTER_R_MAX 1023
#endif
#ifdef KH930
    #define FILTER_L_MIN 200 // below: L Carriage
    #define FILTER_L_MAX 600 // above: K Carriage
    #define FILTER_R_MIN 0
    #define FILTER_R_MAX 600
#endif


/*
 * END OF USERSETTINGS
 */

// DO NOT TOUCH
#define FW_VERSION_MAJ 0
#define FW_VERSION_MIN 90
#define API_VERSION 4 // for message description, see below

#define SERIAL_BAUDRATE 115200

#define BEEPDELAY 50 // ms

// Pin Assignments
#define EOL_PIN_R 0	// Analog
#define EOL_PIN_L 1	// Analog

#define ENC_PIN_A 2
#define ENC_PIN_B 3

#define ENC_PIN_C 4

#define LED_PIN_A 5 // green LED
#define LED_PIN_B 6 // yellow LED

#define PIEZO_PIN 9

#define DBG_BTN_PIN 7   // DEBUG BUTTON

// Machine constants
#define NUM_NEEDLES    200
#define END_LEFT       0
#define END_RIGHT      255
#define START_OFFSET_L 40
#define START_OFFSET_R 16
#define END_OF_LINE_OFFSET_L 32
#define END_OF_LINE_OFFSET_R 12

// Typedefs
#define uint16 unsigned int

typedef enum AYAB_API{
    reqStart_msgid    = 0x01,
    cnfStart_msgid    = 0xC1,
    reqLine_msgid     = 0x82,
    cnfLine_msgid     = 0x42,
    reqInfo_msgid     = 0x03,
    cnfInfo_msgid     = 0xC3,
    reqTest_msgid     = 0x04,
    cnfTest_msgid     = 0xC4,
    indState_msgid    = 0x84,
    debug_msgid       = 0xFF
} AYAB_API_t;

typedef enum Direction{
	NoDirection	= 0,
	Left  		= 1,
	Right 		= 2
} Direction_t;

typedef enum Carriage{
    NoCarriage = 0,
    K        = 1,
    L        = 2
} Carriage_t;

typedef enum Beltshift{
	Unknown     = 0,
	Regular 	= 1, 
	Shifted   	= 2,
    Lace_Regular = 3,
    Lace_Shifted = 4
} Beltshift_t;

typedef enum OpState{
	s_init      = 0,
	s_ready     = 1,
	s_operate   = 2,
    s_test      = 3
} OpState_t;
//


#endif
