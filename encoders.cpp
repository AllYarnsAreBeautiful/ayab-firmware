// encoders.cpp

#include "Arduino.h"
#include "encoders.h"


Encoders::Encoders()
{
	m_direction    = NoDirection;
   m_hallActive   = NoDirection;
	m_beltShift    = Unknown;
	m_encoderPos   = 0x00;
}


void Encoders::encA_interrupt()
{
   m_hallActive = NoDirection;

   static bool _oldState = false;
   bool _curState=digitalRead(ENC_PIN_A);

   if( !_oldState && _curState )
   {
      encA_rising();
   }
   else if( _oldState && !_curState )
   {
      encA_falling();
   }
   _oldState = _curState;

}

/*
 * PRIVATE METHODS
 */ 
void Encoders::encA_rising()
{
	// Direction only decided on rising edge of encoder A
	m_direction = digitalRead(ENC_PIN_B) ? Right : Left;
	/*
   if( Right == m_direction )
	{
		digitalWrite(LED_PIN_B, 0);
	}
	else
	{
		digitalWrite(LED_PIN_B, 1);
	} */

	// Left Hall Sensor
	if( Right == m_direction )
	{
      if( m_encoderPos < END_RIGHT )
      {
         m_encoderPos++;
      }

		uint16 hallValue = analogRead(EOL_PIN_L);
		if( hallValue < FILTER_L_MIN || 
			hallValue > FILTER_L_MAX)
		{ 
         m_hallActive = Left;

			// Belt shift signal only decided in front of hall sensor
			m_beltShift = digitalRead(ENC_PIN_C) ? Regular : Shifted;			

         // Known position of the sled -> overwrite position
         m_encoderPos = END_LEFT + 28;
      }
	}
}


void Encoders::encA_falling()
{
	// Right Hall Sensor
	if( Left == m_direction )
	{
      if( m_encoderPos > END_LEFT )
      {
         m_encoderPos--;
      }
		
      uint16 hallValue = analogRead(EOL_PIN_R);
		if( hallValue < FILTER_R_MIN || 
			hallValue > FILTER_R_MAX)
		{ 
         m_hallActive = Right;

         // Belt shift signal only decided in front of hall sensor
	      m_beltShift = digitalRead(ENC_PIN_C) ? Shifted : Regular;

         // Known position of the sled -> overwrite position
         m_encoderPos = END_RIGHT - 28;
      }
   }
}


byte Encoders::getPosition()
{
	return m_encoderPos;
}


Beltshift_t Encoders::getBeltshift()
{
	return m_beltShift;
}


Direction_t Encoders::getDirection()
{
	return m_direction;
}

Direction_t Encoders::getHallActive()
{
   return m_hallActive;
}


/*
 * PRIVATE METHODS
 */
