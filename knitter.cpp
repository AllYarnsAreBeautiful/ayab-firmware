// knitter.cpp

#include "Arduino.h"
#include "knitter.h"


Knitter::Knitter()
{ 
	m_opState = s_init;

	digitalWrite( LED_PIN_B, 1 ); // indicates ready/operation state

	#ifdef DEBUG
		for( int i = 0; i < 25; i++ )
		{
			//m_currentLine[i] = (i%2) ? 0x00 : 0xFF;
			m_currentLine[i] = 0;

			Serial.print(i);
			Serial.print(": ");
			Serial.println(m_currentLine[i]);
		} 

		m_currentLine[3] = 0xA5;
		m_currentLine[17] = 0x5A;
	#endif //DEBUG 
}


void Knitter::fsm(byte position, Direction_t direction, Beltshift_t beltshift, Direction_t hallActive)
{
	switch( m_opState ) {
		case s_init:
			state_init(direction, hallActive);
			break;

		case s_ready:
			state_ready();
			break;

		case s_operate:
			state_operate(position, direction, beltshift);
			break;

		default: 
			break;
	}
}

bool Knitter::startOperation()
{
	if( s_ready == m_opState )
	{
		m_opState = s_operate;
		return true;
	}
	else
	{
		return false;
	}
}

/*
 * PRIVATE METHODS
 */
void Knitter::state_init(Direction_t direction, Direction_t hallActive)
{
	// Machine is initialized when left hall sensor is passed in Right direction
	if( Right == direction && Left == hallActive )
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
	//DEBUG> jump over state
	m_opState = s_operate;
}


void Knitter::state_operate( byte position, 
					Direction_t direction, 
					Beltshift_t beltshift )
{
	static byte _sOldPosition = 0;
	byte _solenoidToSet = 0;
	byte _pixelToSet    = 0;

	digitalWrite( LED_PIN_B, !digitalRead(LED_PIN_B) );
	digitalWrite( LED_PIN_A, (Shifted == beltshift) );

	if( _sOldPosition != position ) 
	{ // Only act if there is an actual change of position

		if( END_LEFT == position || END_RIGHT == position )
		{ // Turn solenoids off on end of line
			m_solenoids.setSolenoids( 0x0000 );
			return;
		}
		switch( direction )
		{ // Calculate the solenoid and pixel to be set
			case Right:
				if( position >= 40 /*((2*NEEDLE_OFFSET)-24) */ ) 
				{ 
					_pixelToSet = position - 40 /*((2*NEEDLE_OFFSET)-24) */;

					if( Regular == beltshift )
					{
						_solenoidToSet = position % 16;
					}
					else if ( Shifted == beltshift )
					{
						_solenoidToSet = (position-8) % 16;
					}
				}
				else
				{
					return;
				}
				break;

			case Left:
				if( position <= 239 /*(END_RIGHT-((2*NEEDLE_OFFSET)-CAM_OFFSET)) */ )
				{ 
					_pixelToSet = position - 16;
					if( Regular == beltshift )
					{
						_solenoidToSet = (position+8) % 16;
					}
					else if ( Shifted == beltshift )
					{
						_solenoidToSet = position % 16;
					}
				}
				else
				{
					return;
				}
				break;

			default:
				break;
		}
		// Store current Encoder position for next call of this function
		_sOldPosition = position;

		if( _pixelToSet > 199 )
		{
			return;
		}

		#ifdef DEBUG
		Serial.print("PixelToSet: ");
		Serial.println(_pixelToSet);
		Serial.print("SolenoidToSet: ");
		Serial.println(_solenoidToSet);
		#endif

		// Find the right byte from the currentLine array,
		// then read the appropriate Pixel(Bit) for the current needle to set
		int _currentByte = (int)(_pixelToSet/8);

		#ifdef DEBUG
		Serial.print("byte: ");
		Serial.print(_currentByte);
		Serial.print("value: ");
		Serial.println(m_currentLine[_currentByte]);
		#endif

		bool _pixelValue = bitRead( m_currentLine[_currentByte], _pixelToSet-(8*_currentByte) );

		// Write Pixel state to the appropriate needle
		m_solenoids.setSolenoid( _solenoidToSet, _pixelValue );

		// Check position and decide whether to get a new Line from Host
		// TODO 
	}
}
