// solenoids.cpp

#include "Arduino.h"
#include "solenoids.h"

#include <Wire.h>

Solenoids::Solenoids()
{
	#define THIS "Solenoids::Solenoids"

	INFO(THIS,"constructor");

	solenoidState = 0x00;

	Wire.begin();
}


void Solenoids::setSolenoid( byte solenoid, bool state )
{
	#define THIS Solenoids::setSolenoid

	if( solenoid >= 0 && solenoid <= 15 )
	{
		if( state )
			bitSet(solenoidState, solenoid);
		else
			bitClear(solenoidState,solenoid);

		setSolenoids(solenoidState);
	}
	else
		WARNING(THIS,"solenoid number out of range");
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
	#define THIS "Solenoids::write"

	INFO(THIS, "writing to I2C" );

	Wire.beginTransmission( I2Caddr_sol1_8 );
	Wire.write( lowByte(newState) );
	Wire.endTransmission();

	Wire.beginTransmission( I2Caddr_sol9_16);
	Wire.write( highByte(newState) );
	Wire.endTransmission();
}
