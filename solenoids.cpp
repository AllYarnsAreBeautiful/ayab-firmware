// solenoids.cpp

#include "Arduino.h"
#include "solenoids.h"

// Determine board type
#if defined(__AVR_ATmega168__) || defined(__AVR_ATmega328P__)
	// Regular Arduino
	#include <Wire.h>
	#define IIC_HARD
#elif defined(__AVR_ATmega2560__) || defined(__AVR_ATmega1280__)
	// Arduino Mega
	#include "SoftI2CMaster.h"
	SoftI2CMaster Wire(A4,A5,1);
#else
    //untested board - please check your I2C ports
#endif



Solenoids::Solenoids()
{
	solenoidState = 0x00;
	#ifdef IIC_HARD
		Wire.begin();
	#endif //IIC_HARD
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
	Wire.beginTransmission( I2Caddr_sol1_8 );
	Wire.send( lowByte(newState) );
	Wire.endTransmission();

	Wire.beginTransmission( I2Caddr_sol9_16);
	Wire.send( highByte(newState) );
	Wire.endTransmission(); 
}