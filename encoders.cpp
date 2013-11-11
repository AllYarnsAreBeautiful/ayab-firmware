// encoders.cpp

#include "Arduino.h"
#include "encoders.h"


Encoders::Encoders()
{
	m_direction    = NoDirection;
	m_beltShift    = Unknown;
	m_encoderPos   = 0xFF;
}


void Encoders::encA_rising()
{
	INFO(__func__, "");
	// Direction only decided on rising edge of encoder A
	m_direction = digitalRead(ENC_PIN_B) ? Right : Left;

	// Left Hall Sensor
	if( Right == m_direction )
	{
		uint16 hallValue = analogRead(EOL_PIN_L);
		if( hallValue < FILTER_L_MIN || 
			hallValue > FILTER_L_MAX)
		{ 
			// Belt shift signal only decided in rest position
			m_beltShift = digitalRead(ENC_PIN_C) ? Shifted : Regular;
			m_encoderPos = 0; // = Left rest position
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
		uint16 hallValue = analogRead(EOL_PIN_R);
		if( hallValue < FILTER_R_MIN || 
			hallValue > FILTER_R_MAX)
		{ 
	        m_beltShift = digitalRead(ENC_PIN_C) ? Regular : Shifted;
	        m_encoderPos = 201; // = Right rest position
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


Direction_t Encoders::getDirection()
{
	return m_direction;
}


/*
 * PRIVATE METHODS
 */
