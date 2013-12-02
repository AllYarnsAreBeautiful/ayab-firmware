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

void Knitter::isr()
{
	// Update machine state data
	m_encoders.encA_interrupt();
	m_position   = m_encoders.getPosition();
	m_direction  = m_encoders.getDirection();
	m_hallActive = m_encoders.getHallActive();
	m_beltshift  = m_encoders.getBeltshift();
}

void Knitter::fsm()
{
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

bool Knitter::startOperation(byte startNeedle, byte stopNeedle, byte (*line))
{
	if( s_ready == m_opState )
	{
		m_startNeedle 	= startNeedle;
		m_stopNeedle  	= stopNeedle;
		m_currentLineNumber	= 255; // necessary to start with line 0
		m_lineRequested = false;
		m_opState 	  	= s_operate;
		m_firstLineFlag	= true;
		m_lastLineFlag		= false;
		m_lastLinesCountdown= 2;
		m_currentLine 		= line;
		m_nextLine 			= line;

/*
		Serial.write(0xFF);
		Serial.print("#startNeedle: ");
		Serial.print(startNeedle);
		Serial.print(" stopNeedle: ");
		Serial.println(stopNeedle); */
		return true;

		// TODO request 1 or 2 lines immediately?
	}
	else
	{
		return false;
	}
}


bool Knitter::setNextLine(byte lineNumber, byte (*line))
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
			return true;
		}
		else
		{	// line numbers didnt match -> request again
			reqLine(m_currentLineNumber);
		}
	}
	return false;
}


void Knitter::setLastLine()
{
	m_lastLineFlag = true;
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
		m_solenoids.setSolenoids(0xFFFF);
	}
	else
	{
		m_opState = s_init;
	}
}


void Knitter::state_ready()
{
	digitalWrite(LED_PIN_A,0);	
	// This state is left by the startOperation() method called by main
}


void Knitter::state_operate()
{
	digitalWrite(LED_PIN_A,1);
	static byte _sOldPosition = 0;
	static bool _workedOnLine = false; // Necessary to start with a line request

	if( _sOldPosition != m_position ) 
	{ // Only act if there is an actual change of position
		// Store current Encoder position for next call of this function
		_sOldPosition = m_position;	

		
		if( !calculatePixelAndSolenoid() )
		{
			// No valid/useful position calculated
			return;
		}

		if( (m_pixelToSet >= m_startNeedle-20)
				&& (m_pixelToSet <= m_stopNeedle+20)) // TODO ADD OFFSET
		{	// When inside the active needles
			digitalWrite(LED_PIN_B, 1);
			_workedOnLine = true;

			// Find the right byte from the currentLine array,
			// then read the appropriate Pixel(/Bit) for the current needle to set
			int _currentByte = (int)(m_pixelToSet/8);
			bool _pixelValue = bitRead( m_currentLine[_currentByte], 
										m_pixelToSet-(8*_currentByte) );
			// Write Pixel state to the appropriate needle
			m_solenoids.setSolenoid( m_solenoidToSet, _pixelValue );

			/*
			#ifdef DEBUG
			Serial.print("PixelToSet: ");
			Serial.print(m_pixelToSet);
			Serial.print(" SolenoidToSet: ");
			Serial.print(m_solenoidToSet);
			Serial.print(" - byte: ");
			Serial.print(_currentByte);
			Serial.print(" value: ");
			Serial.println(m_currentLine[_currentByte]);
			#endif */
		}
		else
		{	// Outside of the active needles
			digitalWrite(LED_PIN_B, 0);
			if( _workedOnLine )
			{	// already worked on the current line -> finished the line
				_workedOnLine   = false;
			/*	Serial.write(0xFF);
				Serial.println("#finished Line! ");*/

				if( m_firstLineFlag )
				{
					m_firstLineFlag = false;
				}
				else
				{
					// load nextLine pointer
					m_currentLine = m_nextLine;
				}

				if( !m_lineRequested && !m_lastLineFlag )
				{	// request new Line from Host	
					reqLine(++m_currentLineNumber);					
				}
				else if( m_lastLineFlag )
				{
					m_lastLinesCountdown--;
					if(0 == m_lastLinesCountdown)
					{	// All lines are processed, go back to ready state
						m_beeper.endWork();
						m_opState = s_ready;		
						m_solenoids.setSolenoids(0xFFFF);				
					}
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