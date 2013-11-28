// settings.h

#ifndef SETTINGS_H
#define SETTINGS_H

#define VERSION_STRING "01.00"

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

#define NEEDLE_OFFSET 28
#define CAM_OFFSET    28


#define END_LEFT  0
#define END_RIGHT 255

#define uint16 unsigned int

typedef enum Direction{
	NoDirection	= 0,
	Left  		= 1,
	Right 		= 2
} Direction_t;

typedef enum Beltshift{
	Unknown     = 0,
	Regular 	   = 1, 
	Shifted   	= 2
} Beltshift_t;

typedef enum OpState{
	s_init      = 0,
	s_ready     = 1,
	s_operate   = 2
} OpState_t;


#endif