// knitter.cpp

#include "Arduino.h"
#include "knitter.h"


Knitter::Knitter()
{ 
	m_opState           = s_init;
	m_startNeedle       = 0;
	m_stopNeedle        = 0;
	m_currentLineNumber = 0;
	m_lineRequested     = false;
}


void Knitter::fsm()
{
	// Update machine state data
	m_encoders.encA_interrupt();
	m_position   = m_encoders.getPosition();
	m_direction  = m_encoders.getDirection();
	m_hallActive = m_encoders.getHallActive();
	m_beltshift  = m_encoders.getBeltshift();

	switch( m_opState ) {
		case s_init:
			state_init();
			break;

		case s_ready:
			state_ready();
			break;

		case s_operate:
			state_operate();
			break;

		default: 
			break;
	}
}

bool Knitter::startOperation(byte startNeedle, byte stopNeedle)
{
	if( s_ready == m_opState )
	{
		m_startNeedle 	= startNeedle;
		m_stopNeedle  	= stopNeedle;
		m_currentLineNumber	= 0;
		m_lineRequested = false;
		m_opState 	  	= s_operate;
		return true;

		// TODO request 1 or 2 lines immediately?
	}
	else
	{
		return false;
	}
}


void Knitter::setNextLine(byte lineNumber, byte (*line))
{
	if( m_lineRequested )
	{	// Is there even a need for a new line?
		if( lineNumber == m_currentLineNumber )
		{
			m_lineRequested = false;
			
			// Set nextLine pointer to buffer
			m_nextLine = line;

			m_beeper.ready();
			
			/* #ifdef DEBUG
			for(int i = 0; i < 25; i++)
			{
				Serial.println(m_nextLine[i]);
			}
			#endif */
		}
		else
		{	// line numbers didnt match -> request again
			reqLine(m_currentLineNumber);
		}
	}
}


void Knitter::endWork()
{
	m_opState = s_ready;
	
	m_beeper.endWork();
}


/*
 * PRIVATE METHODS
 */
void Knitter::state_init()
{
	// Machine is initialized when left hall sensor is passed in Right direction
	if( Right == m_direction && Left == m_hallActive )
	{
		m_opState = s_ready;
	}
	else
	{
		m_opState = s_init;
	}
}


void Knitter::state_ready()
{
	// This state is left by the startOperation() method called by main
}


void Knitter::state_operate()
{
	static byte _sOldPosition = 0;
	static bool _workedOnLine = false;

	if( _sOldPosition != m_position ) 
	{ // Only act if there is an actual change of position
		digitalWrite( LED_PIN_B, !digitalRead(LED_PIN_B) );
		// Store current Encoder position for next call of this function
		_sOldPosition = m_position;	

		
		if( !calculatePixelAndSolenoid() )
		{
			// No valid/useful position calculated
			return;
		}

		if( (m_pixelToSet >= m_startNeedle)
				&& (m_pixelToSet <= m_stopNeedle))
		{	// When inside the active needles
			_workedOnLine = true;

			// Find the right byte from the currentLine array,
			// then read the appropriate Pixel(/Bit) for the current needle to set
			int _currentByte = (int)(m_pixelToSet/8);
			bool _pixelValue = bitRead( m_currentLine[_currentByte], 
										m_pixelToSet-(8*_currentByte) );
			// Write Pixel state to the appropriate needle
			m_solenoids.setSolenoid( m_solenoidToSet, _pixelValue );

			#ifdef DEBUG
			Serial.print("PixelToSet: ");
			Serial.print(m_pixelToSet);
			Serial.print(" SolenoidToSet: ");
			Serial.print(m_solenoidToSet);
			Serial.print(" - byte: ");
			Serial.print(_currentByte);
			Serial.print(" value: ");
			Serial.println(m_currentLine[_currentByte]);
			#endif
		}
		else
		{	// Outside of the active needles
			// Turn solenoids off
			// TODO verify if this is ok here
			m_solenoids.setSolenoids( 0x0000 );
			
			if( _workedOnLine )
			{	// already worked on the current line -> finished the line
				m_beeper.finishedLine();

				// load nextLine pointer
				m_currentLine = m_nextLine;
				
				if( !m_lineRequested )
				{	// request new Line from Host	
					reqLine(++m_currentLineNumber);

					_workedOnLine   = false;
				}
			}
		}
	}
}


bool Knitter::calculatePixelAndSolenoid()
{
	switch( m_direction )
	{ // Calculate the solenoid and pixel to be set
	  // Implemented according to machine manual
	  // Magic numbers result from manual	
		case Right:
			if( m_position >= START_OFFSET_L ) 
			{ 
				m_pixelToSet = m_position - START_OFFSET_L;
				
				if( Regular == m_beltshift )
				{
					m_solenoidToSet = m_position % 16;
				}
				else if ( Shifted == m_beltshift )
				{
					m_solenoidToSet = (m_position-8) % 16;
				}
			}
			else
			{
				return false;
			}
			break;

		case Left:
			if( m_position <= (END_RIGHT - START_OFFSET_R) )
			{ 
				m_pixelToSet = m_position - START_OFFSET_R;
				if( Regular == m_beltshift )
				{
					m_solenoidToSet = (m_position+8) % 16;
				}
				else if ( Shifted == m_beltshift )
				{
					m_solenoidToSet = m_position % 16;
				}
			}
			else
			{
				return false;
			}
			break;

		default:
			return false;
			break;
	}
	return true;
}


void Knitter::reqLine( byte lineNumber )
{
	Serial.write(0x82);
	Serial.write(lineNumber);
	Serial.println("");

	m_lineRequested = true;
}