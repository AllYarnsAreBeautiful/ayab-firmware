// encoders.cpp

#include "Arduino.h"
#include "encoders.h"


Encoders::Encoders()
{
	m_direction    = NoDirection;
	m_beltShift    = NoPhase;
	m_encoderPos   = 0xFF;
}


void Encoders::encA_rising()
{
	INFO(__func__, "");
	m_direction = digitalRead(ENC_PIN_B) ? Right : Left;

	// Left Hall Sensor
	if( Right == m_direction )
	{
		INFO(__func__, "Right direction");
		uint16 hallValue = analogRead(EOL_PIN_L);
		if( hallValue < FILTER_L_MIN || 
			hallValue > FILTER_L_MAX)
		{ 
			INFO(__func__, "Left rest position");
			m_beltShift = digitalRead(ENC_PIN_C) ? Goofy : Regular;
			m_encoderPos = 0; // 0 = Left reset position
      	}
      	else
      	{
      		m_encoderPos++;
      	}
	}
}


void Encoders::encA_falling()
{
	INFO(__func__, "");
	// Right Hall Sensor
	if( Left == m_direction )
	{
		INFO(__func__, "Left direction");
		uint16 hallValue = analogRead(EOL_PIN_R);
		if( hallValue < FILTER_R_MIN || 
			hallValue > FILTER_R_MAX)
		{ 
			INFO(__func__, "Right rest position");
	        m_beltShift = digitalRead(ENC_PIN_C) ? Regular : Goofy;
	        m_encoderPos = 201; // 201 = Right reset position
      	}
      	else
      	{
      		m_encoderPos--;
      	}
	}
}


byte Encoders::getPosition()
{
	return m_encoderPos;
}


Phaseshift_t Encoders::getPhaseshift()
{
	return m_beltShift;
}


/*
 * PRIVATE METHODS
 */
