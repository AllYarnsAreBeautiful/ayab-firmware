// solenoids.cpp

#include "Arduino.h"
#include "solenoids.h"

#include "SoftI2CMaster.h"

SoftI2CMaster i2cMaster(A4,A5,1);

Solenoids::Solenoids()
{
	solenoidState = 0x00;
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
	i2cMaster.beginTransmission( I2Caddr_sol1_8 );
	i2cMaster.send( lowByte(newState) );
	i2cMaster.endTransmission();

	i2cMaster.beginTransmission( I2Caddr_sol9_16);
	i2cMaster.send( highByte(newState) );
	i2cMaster.endTransmission(); 
}
