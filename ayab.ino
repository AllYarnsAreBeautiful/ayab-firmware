// ayab.ino

/*
 * INCLUDES
 */
#include "Arduino.h"
#include "SerialCommand.h"

#include "debug.h"
#include "settings.h"

#include "beeper.h"
#include "encoders.h"
#include "knitter.h"

/*
 * DEFINES
 */

/*
 *	DECLARATIONS
 */ 
Beeper        beeper;
Encoders      encoders;
Knitter		  *knitter;

byte needlePos, oldNeedlePos;

/*
 * SETUP
 */
void setup() {
  Serial.begin(SERIAL_BAUDRATE);

  pinMode(ENC_PIN_A,INPUT);
  pinMode(ENC_PIN_B,INPUT);
  pinMode(ENC_PIN_C,INPUT);

  pinMode(LED_PIN_A,OUTPUT);
  pinMode(LED_PIN_B,OUTPUT); 

  //Attaching ENC_PIN_A(=2), Interrupt No. 0
  attachInterrupt(0, isr_encA_rising, RISING);
  attachInterrupt(0, isr_encA_falling, FALLING);

  DEBUG_PRINT("ayab ready");

  knitter = new Knitter();
}


void loop() {
	#ifdef DEBUG
		needlePos = encoders.getPosition();
		if ( oldNeedlePos != needlePos )
		{
			Serial.print("Needle Position: ");
			Serial.println(needlePos);
			Serial.print("BeltShift: ");
			Serial.println( encoders.getPhaseshift() );
		}
		oldNeedlePos = needlePos;
	#endif
}


void isr_encA_rising()
{
	encoders.encA_rising();
}

void isr_encA_falling()
{
	encoders.encA_falling();
}


/*
 * Serial Command handling
 */

 void h_reqInit()
 {
 	DEBUG_PRINT("");
 }

 void h_reqStart()
 {
 	DEBUG_PRINT("");
 }

 void h_cnfLine()
 {
 	DEBUG_PRINT("");
 }

 void h_reqStop()
 {
 	DEBUG_PRINT("");
 }

 void h_reqInfo()
 {
 	DEBUG_PRINT("");
 	Serial.print(0xC5); //cnfInfo
 	Serial.print(" ");
 	Serial.println(VERSION_STRING);
 }

void h_unrecognized()
{
	DEBUG_PRINT("cmd unrecognized");
}
