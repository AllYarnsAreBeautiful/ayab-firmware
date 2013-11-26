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

	void knit(byte position, 
				Direction_t direction, 
				Phaseshift_t phaseshift);

private:
	Solenoids   m_solenoids;

	byte		m_currentLine[25];

	byte 		m_oldPosition;
	byte		m_solenoidToSet;
	byte		m_pixelToSet;
};


#endif // KNITTER_H