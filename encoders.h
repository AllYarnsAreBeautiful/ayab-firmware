// encoders.h

#ifndef ENCODERS_H
#define ENCODERS_H

#include "Arduino.h"
#include "settings.h"
#include "debug.h"

class Encoders{
public:
	Encoders();

   void encA_interrupt();

	byte 			getPosition();
	Beltshift_t 	getBeltshift();
	Direction_t 	getDirection();
   Direction_t    getHallActive();

private:
	Direction_t 	m_direction;
   Direction_t    m_hallActive;
	Beltshift_t 	m_beltShift;
	byte 			m_encoderPos;

   void encA_rising();
   void encA_falling();
};

#endif