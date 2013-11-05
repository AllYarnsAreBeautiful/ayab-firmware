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
	#define THIS "Beeper::beep"
	
    for(int i=0;i<length;i++)
    {
      analogWrite(PIEZO_PIN, 20);      // Almost any value can be used except 0 and 255
      // experiment to get the best tone
      delay(DELAYMS);          // wait for a delayms ms
      analogWrite(PIEZO_PIN, 0);       // 0 turns it off
      delay(DELAYMS);          // wait for a delayms ms 
    }
}