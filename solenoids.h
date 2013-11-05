// solenoids.h

#ifndef SOLENOIDS_H
#define SOLENOIDS_H

#include "Arduino.h"

#include "settings.h"
#include "debug.h"


#define I2Caddr_sol1_8  0x20
#define I2Caddr_sol9_16 0x21


class Solenoids
{

public:
	Solenoids();
	void setSolenoid( byte solenoid, bool state);
	void setSolenoids( uint16 state );

private:
	uint16 solenoidState;
	void write( uint16 state );

};

#endif