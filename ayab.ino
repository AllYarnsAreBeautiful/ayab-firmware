// ayab.ino

/*
 * INCLUDES
 */
#include "Arduino.h"
#include "SerialCommand.h"

#include "debug.h"
#include "settings.h"

#include "beeper.h"
#include "solenoids.h"
#include "encoders.h"

/*
 * DEFINES
 */

/*
 *	DECLARATIONS
 */ 
Beeper        beeper;
Solenoids     solenoids;
Encoders      encoders;

byte needlePos, oldNeedlePos;

/*
 * SETUP
 */
void setup() {
  Serial.begin(SERIAL_BAUDRATE);

  pinMode(ENC_PIN_A,INPUT);
  pinMode(ENC_PIN_B,INPUT);
  pinMode(ENC_PIN_C,INPUT);

  attachInterrupt(ENC_PIN_A, isr_encA_rising, RISING);
  attachInterrupt(ENC_PIN_A, isr_encA_falling, FALLING);

  INFO(__func__,"ready");
}


void loop() {
	needlePos = encoders.getPosition();
	if ( oldNeedlePos != needlePos )
	{
		INFO(__func__, "Needle Position:");
		INFO(__func__, needlePos);
		INFO(__func__, "BeltShift:");
		INFO(__func__, getPhaseshift() );
	}
	oldNeedlePos = needlePos;
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

 void handle_reqInit()
 {
 	INFO(__func__,"");
 }

 void handle_reqStart()
 {
 	INFO(__func__,"");
 }

 void handle_cnfLine()
 {
 	INFO(__func__,"");
 }

 void handle_reqStop()
 {
 	INFO(__func__,"");
 }

 void handle_reqInfo()
 {
 	INFO(__func__,"");
 }

void handle_unrecognized()
{
	INFO(__func__,"cmd unrecognized");
}
