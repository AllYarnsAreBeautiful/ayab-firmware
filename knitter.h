// knitter.h

#ifndef KNITTER_H
#define KNITTER_H

#include "Arduino.h"
#include "settings.h"
#include "debug.h"

#include "solenoids.h"

typedef byte Line[25];

class Knitter
{
public:
	Knitter();

	void knit(byte position, 
				Direction_t direction, 
				Phaseshift_t phaseshift);

private:
	Solenoids   m_solenoids;

	Line		m_currentLine;

	byte 		m_oldPosition;
	byte		m_solenoidToSet;
	byte		m_pixelToSet;
};


#endif // KNITTER_H