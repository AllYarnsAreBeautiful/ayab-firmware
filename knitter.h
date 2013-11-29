// knitter.h

#ifndef KNITTER_H
#define KNITTER_H

#include "Arduino.h"
#include "settings.h"
#include "debug.h"

#include "solenoids.h"
#include "encoders.h"
#include "beeper.h"

class Knitter
{
public:
	Knitter();

	void fsm();
	bool startOperation(byte startNeedle,
						byte stopNeedle);
	void setNextLine(byte lineNumber, byte (*line));
	void endWork();

private:
	Solenoids   m_solenoids;
	Encoders    m_encoders;
	Beeper      m_beeper;

	OpState_t   m_opState;

	// Job Parameters
	byte		m_startNeedle;
	byte		m_stopNeedle;
	bool		m_lineRequested;
	uint16 		m_currentLineNumber;
	byte		(*m_currentLine);
	byte		(*m_nextLine);

	// current machine state
	byte 		m_position;
	Direction_t	m_direction;
	Direction_t m_hallActive;
	Beltshift_t m_beltshift;

	// Resulting needle data
	byte 		m_solenoidToSet;
	byte 		m_pixelToSet;


	void state_init();
	void state_ready();
	void state_operate();

	bool calculatePixelAndSolenoid();

	void reqLine( byte lineNumber );
};


#endif // KNITTER_H