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
			m_currentLine[i] = (i%2) ? 0xFF : 0x00;
			Serial.print(i);
			Serial.print(": ");
			Serial.println(m_currentLine[i]);
		}
	#endif //DEBUG 
}


void Knitter::fsm(byte position, Direction_t direction, Beltshift_t beltshift)
{
	switch( m_opState ) {
		case s_init:
			state_init(position);
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
void Knitter::state_init(byte position)
{
	digitalWrite( LED_PIN_B, 1 );

	static Direction_t _sInitialPos = NoDirection;

	// Check where the sled was inserted (Left or right of the middle)
	if( _sInitialPos == NoDirection )
	{
		_sInitialPos = position < 127 ? Left : Right;
	}

	// Check if the sled has reached the other side of the machine
	// Human has to make sure the sled is moved to the very end
	if( (_sInitialPos == Left && position == END_RIGHT )
		|| ( _sInitialPos == Right && position == END_LEFT ) )
	{
		// Reset variable and move to next state
		_sInitialPos   = NoDirection;
		m_opState    = s_ready;
	}

}


void Knitter::state_ready()
{
	digitalWrite( LED_PIN_B, 0 );
	// This state is left by the startOperation() method called by main
}


void Knitter::state_operate( byte position, 
					Direction_t direction, 
					Beltshift_t beltshift )
{
	static byte _sOldPosition = 0;
	byte _solenoidToSet = 0;
	byte _pixelToSet    = 0;

	digitalWrite( LED_PIN_B, !digitalRead(LED_PIN_B) );

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
				if( position > NEEDLE_OFFSET )
				{ 
					_pixelToSet = position - NEEDLE_OFFSET; // TODO CHECK!
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
					return;
				break;

			case Left:
				if( position < (END_RIGHT-NEEDLE_OFFSET) )
				{
					_pixelToSet = position + NEEDLE_OFFSET; // TODO CHECK!
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
					return;
				break;

			default:
				break;
		}

		// Find the right byte from the currentLine array,
		// then read the appropriate Pixel(Bit) for the current needle to set
		bool _pixelValue = bitRead( m_currentLine[(int)(_pixelToSet/8)], _solenoidToSet );
		// Write Pixel state to the appropriate needle
		m_solenoids.setSolenoid( _solenoidToSet, _pixelValue );

		// Store current Encoder position for next call of this function
		_sOldPosition = position;

		// Check position and decide whether to get a new Line from Host
		// TODO
	}
}
