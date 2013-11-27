// knitter.h

#ifndef KNITTER_H
#define KNITTER_H

#include "Arduino.h"
#include "settings.h"
#include "debug.h"

#include "solenoids.h"

class Knitter
{
public:
	Knitter();

	void fsm(byte position, 
		Direction_t direction, 
		Beltshift_t beltshift);
	bool startOperation();

private:
	Solenoids   m_solenoids;

	OpState_t   m_opState;

	byte		m_currentLine[25];

	void state_init(byte position);
	void state_ready(void);
	void state_operate(byte position, 
				Direction_t direction, 
				Beltshift_t beltshift);
};


#endif // KNITTER_H