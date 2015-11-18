// solenoids.cpp
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
#include "solenoids.h"

// Determine board type
#if defined(__AVR_ATmega168__) || defined(__AVR_ATmega328P__)
  // Regular Arduino
  #warning Using Hardware I2C
  #ifndef HARD_I2C
    #define HARD_I2C
  #endif
  #include <Wire.h>
  #include "Adafruit_MCP23008.h"

  Adafruit_MCP23008 mcp_0;
  Adafruit_MCP23008 mcp_1;
#elif defined(__AVR_ATmega2560__) || defined(__AVR_ATmega1280__)
  // Arduino Mega
  #warning Using Software I2C
  #ifndef SOFT_I2C
    #define SOFT_I2C
  #endif
  #include <SoftI2CMaster.h>
  SoftI2CMaster Wire(A4,A5,1);
#else
  #warning untested board - please check your I2C ports
#endif

Solenoids::Solenoids()
{
	solenoidState = 0x00;
}

void Solenoids::init(void)
{
  #ifdef HARD_I2C
    mcp_0.begin(I2Caddr_sol1_8);
    mcp_1.begin(I2Caddr_sol9_16);

    for (int i = 0; i<8; i++)
    {
        mcp_0.pinMode(i, OUTPUT);
        mcp_1.pinMode(i, OUTPUT);
    }
  #endif
  // No Action needed for SOFT_I2C
}

void Solenoids::setSolenoid( byte solenoid, bool state )
{ 	
	if( solenoid >= 0 && solenoid <= 15 )
	{ 
		if( state )
		{
			bitSet(solenoidState, solenoid);
		}
		else
		{
			bitClear(solenoidState,solenoid);
		}	
		// TODO optimize to act only when there is an actual change of state
		write(solenoidState); 
	}
}


void Solenoids::setSolenoids( uint16 state )
{
	solenoidState = state;
	write( state );
}


/*
 * Private Methods
 */


/*
 * Writes to the I2C port expanders
 * Low level function, mapping to actual wiring
 * is done here.
 */
void Solenoids::write( uint16 newState )
{
  #ifdef HARD_I2C
    mcp_0.writeGPIO(lowByte(newState));
    mcp_1.writeGPIO(highByte(newState));
  #elif defined SOFT_I2C
    Wire.beginTransmission( I2Caddr_sol1_8 | 0x20 );
    Wire.send( lowByte(newState) );
    Wire.endTransmission();
    Wire.beginTransmission( I2Caddr_sol9_16 | 0x20);
    Wire.send( highByte(newState) );
    Wire.endTransmission(); 
  #endif
}