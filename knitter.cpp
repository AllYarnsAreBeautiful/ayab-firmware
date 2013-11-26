// knitter.cpp

#include "Arduino.h"
#include "knitter.h"


Knitter::Knitter()
{ 
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

void Knitter::knit( byte position, 
					Direction_t direction, 
					Phaseshift_t phaseshift )
{
	if( m_oldPosition != position )
	{ // Only act if there is an actual change of position

		if( 0 == position || 255 == position )
		{ // Turn solenoids off on end of line
			m_solenoids.setSolenoids( 0x0000 );
			return;
		}

		switch( direction )
		{ // Calculate the solenoid and pixel to be set
			case Right:
				if( position > NEEDLE_OFFSET )
				{ 
					m_pixelToSet = position - NEEDLE_OFFSET; // TODO CHECK!
					if( Regular == phaseshift )
					{
						m_solenoidToSet = position % 16;
					}
					else if ( Shifted == phaseshift )
					{
						m_solenoidToSet = (position-8) % 16;
					}
				}
				else
					return;
				break;

			case Left:
				if( position < (255-NEEDLE_OFFSET) )
				{
					m_pixelToSet = position + NEEDLE_OFFSET; // TODO CHECK!
					if( Regular == phaseshift )
					{
						m_solenoidToSet = (position+8) % 16;
					}
					else if ( Shifted == phaseshift )
					{
						m_solenoidToSet = position % 16;
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
		bool _pixelValue = bitRead( m_currentLine[(int)(m_pixelToSet/8)], m_solenoidToSet );
		// Write Pixel state to the appropriate needle
		m_solenoids.setSolenoid( m_solenoidToSet, _pixelValue );

		// Store current Encoder position for next call of this function
		m_oldPosition = position;

		// Check position and decide whether to get a new Line from Host
		// TODO
	}
}
