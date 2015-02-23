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

// Determine board type
#if defined(__AVR_ATmega168__) || defined(__AVR_ATmega328P__)
  // Regular Arduino
  #include <Wire.h>
#elif defined(__AVR_ATmega2560__) || defined(__AVR_ATmega1280__)
  // Arduino Mega
  #include <SoftI2CMaster.h>
#else
    //untested board - please check your I2C ports
#endif

/*
 * USERSETTINGS
 */

//#define DBG_NOMACHINE  // Turn on to use DBG_BTN as EOL Trigger

#define KH910
//#define KH930

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

#define DBG_BTN_PIN 7   // DEBUG BUTTON

// Machine constants
#define NUM_NEEDLES    200
#define END_LEFT       0
#define END_RIGHT      255
#define START_OFFSET_L 40
#define START_OFFSET_R 16
#define END_OF_LINE_OFFSET_L 32
#define END_OF_LINE_OFFSET_R 12

#define uint16 unsigned int

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
	s_operate   = 2
} OpState_t;
//


#endif