// beeper.cpp

#include "Arduino.h"
#include "beeper.h"

Beeper::Beeper()
{

}


void Beeper::start()
{
	beep(3);
}


void Beeper::endline()
{
	beep(5);
}


/*
 * PRIVATE METHODS
 */
void Beeper::beep( byte length )
{	
    for(int i=0;i<length;i++)
    {
      analogWrite(PIEZO_PIN, 0);
      delay(BEEPDELAY);
      analogWrite(PIEZO_PIN, 20);       
      delay(BEEPDELAY);
    }
    analogWrite(PIEZO_PIN, 255);
}