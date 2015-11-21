// knitter.h
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

	void isr();
	void fsm();
	bool startOperation(byte startNeedle,
						byte stopNeedle,
						byte (*line));
    bool startTest(void);
	bool setNextLine(byte lineNumber);
	void setLastLine();

private:
	Solenoids   m_solenoids;
	Encoders    m_encoders;
	Beeper      m_beeper;

	OpState_t   m_opState;
	bool		m_lastLineFlag;
	byte		m_lastLinesCountdown;

	// Job Parameters
	byte		m_startNeedle;
	byte		m_stopNeedle;
	bool		m_lineRequested;
	byte 		m_currentLineNumber;
	byte		(*m_lineBuffer);

	// current machine state
	byte 		m_position;
	Direction_t	m_direction;
	Direction_t m_hallActive;
	Beltshift_t m_beltshift;
  Carriage_t  m_carriage;

	// Resulting needle data
	byte 		m_solenoidToSet;
	byte 		m_pixelToSet;


	void state_init();
	void state_ready();
	void state_operate();
    void state_test();

	bool calculatePixelAndSolenoid();

	void reqLine( byte lineNumber );
    void indState( bool initState = false);
};


#endif // KNITTER_H