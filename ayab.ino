// ayab.ino

/*
 * INCLUDES
 */
#include "Arduino.h"
#include "SerialCommand.h"

#include "debug.h"
#include "settings.h"

#include "knitter.h"

/*
 * DEFINES
 */

/*
 *	DECLARATIONS
 */ 
Knitter		  *knitter;
byte        lineBuffer[LINE_BUF_SIZE][25];

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
  digitalWrite(LED_PIN_A, 1);
  digitalWrite(LED_PIN_B, 1);

  //Attaching ENC_PIN_A(=2), Interrupt No. 0
  attachInterrupt(0, isr_encA, CHANGE);

  resetLineBuffer();

  knitter = new Knitter();
  
  DEBUG_PRINT("# AYAB ready");
}


void loop() {   
	#ifdef DEBUG
  static byte oldNeedlePos;
	byte needlePos = encoders.getPosition();
	if ( oldNeedlePos != needlePos )
	{
		Serial.print("Needle Position: ");
		Serial.print(needlePos);
		Serial.print(" BeltShift: ");
		Serial.println( encoders.getBeltshift() );
	}
	oldNeedlePos = needlePos;
	#endif
}


void isr_encA()
{   
   knitter->fsm(); 
}


void resetLineBuffer()
{
    // TODO verify operation
    memset(lineBuffer,0,sizeof(lineBuffer));
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
