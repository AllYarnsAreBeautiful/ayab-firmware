// encoders.cpp
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
