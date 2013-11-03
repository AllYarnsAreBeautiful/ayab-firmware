// Solenoids.cpp

#include "Arduino.h"
#include "Solenoids.h"

#include <Wire.h>

Solenoids::Solenoids()
{
	INFO("solenoids","constructor");

	solenoidState = 0x00;

	Wire.begin();
}


void Solenoids::setSolenoid( byte solenoid, bool state )
{
	if( state )
		bitSet(solenoidState, solenoid);
	else
		bitClear(solenoidState,solenoid);

	setSolenoid(solenoidState);
}


void Solenoids::setSolenoid( uint16 state )
{
	solenoidState = state;
	write( lowByte(state), highByte(state) );
}


/*
 * Private Methods
 */


/*
 * Writes to the I2C port expanders
 */
void Solenoids::write( byte sol1_8, byte sol9_16 )
{
	INFO( "solenoids:write", "writing to I2C" );

	Wire.beginTransmission( I2Caddr_sol1_8 );
	Wire.write( sol1_8 );
	Wire.endTransmission();

	Wire.beginTransmission( I2Caddr_sol9_16);
	Wire.write( sol9_16 );
	Wire.endTransmission();
}
