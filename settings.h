// settings.h

#ifndef SETTINGS_H
#define SETTINGS_H

#define VERSION_STRING "01.00"

#define SERIAL_BAUDRATE 115200

#define EOL_PIN_R 0	// Analog
#define EOL_PIN_L 1	// Analog

#define ENC_PIN_A 2
#define ENC_PIN_B 3
#define ENC_PIN_C 4

#define PIEZO_PIN 9

#define FILTER_L_MIN 10
#define FILTER_L_MAX 460
#define FILTER_R_MIN 10
#define FILTER_R_MAX 460

#define NEEDLE_OFFSET 15

#define BEEPDELAY 50 // ms


#define uint16 unsigned int

typedef enum Direction{
	NoDirection	= 0,
	Left  		= 1,
	Right 		= 2
} Direction_t;

typedef enum Phaseshift{
	Unknown     = 0,
	Regular 	= 1, 
	Shifted   	= 2
} Phaseshift_t;


#endif