// ayab_hw_test.ino


/*
 * INCLUDES
 */
#include "Arduino.h"
#include "SerialCommand.h"

#include "debug.h"
#include "settings.h"

#include "beeper.h"
#include "solenoids.h"



/*
 * DEFINES
 */

/*
 *	DECLARATIONS
 */ 
SerialCommand SCmd;   // The SerialCommand object
Beeper        beeper;
Solenoids     solenoids;

bool bInterruptEnabled = false;
/*
 * SETUP
 */
void setup() {

	Serial.begin(9600);

  pinMode(ENC_PIN_A,INPUT);
  pinMode(ENC_PIN_B,INPUT);
  pinMode(ENC_PIN_C,INPUT);

	// Setup callbacks for SerialCommand commands
  SCmd.addCommand("setSingleSolenoid", setSingleSolenoid);
  SCmd.addCommand("setSolenoids", setSolenoids);
  SCmd.addCommand("readEOLsensors", readEOLsensors);
  SCmd.addCommand("readEncoders", readEncoders);
  SCmd.addCommand("beep", beep);
  SCmd.addCommand("setEncoderInterrupt", setEncoderInterrupt);
  SCmd.addCommand("autoRead", autoRead);
	SCmd.addDefaultHandler(unrecognized);  // Handler for command that isn't matched 
	
  attachInterrupt(ENC_PIN_A, encoderAChange, CHANGE);

  INFO(__func__, "ready");
}


/*
 *	MAIN LOOP
 */ 
void loop() {
	SCmd.readSerial(); 
}


/*
 * 
 */
void encoderAChange()
{
  if(bInterruptEnabled)
    INFO(__func__, "INTERRUPT");
}


/*
 *
 */
void setSingleSolenoid()    
{
  int aNumber;  
  char *arg;

  byte solenoidNumber = 0;
  byte solenoidState  = 0;

  INFO(__func__,""); 
  arg = SCmd.next(); 
  if (arg != NULL) 
  {
    solenoidNumber=atoi(arg);
    arg = SCmd.next();
    if(arg != NULL)
    {
      solenoidState = atoi(arg);
      if( 0 == solenoidState || 1 == solenoidState )
      {
        solenoids.setSolenoid( solenoidNumber, solenoidState );
      }
      else
      {
        WARNING(__func__,"invalid arguments");
      }
    }
  }
}


/*
 *
 */
void setSolenoids()
{
  int aNumber;  
  char *arg;

  byte solenoidState  = 0;

  INFO(__func__,""); 
  arg = SCmd.next(); 

  if(arg != NULL)
  {
    solenoidState = atoi(arg);
    if( 0 == solenoidState || 1 == solenoidState )
    {
      solenoids.setSolenoids( solenoidState );
    }
    else
    {
      WARNING(__func__,"invalid arguments");
    }
  }
}


/*
 *
 */
void readEOLsensors()
{
  INFO(__func__, "EOL_R");
  INFO(__func__, atoi(analogRead(EOL_PIN_R)) );
  INFO(__func__, "EOL_L");
  INFO(__func__, atoi(analogRead(EOL_PIN_L)) );
}


/*
 *
 */
void readEncoders()
{
  if( HIGH == digitalRead(ENC_PIN_A) )
  {
    INFO(__func__, "ENC_A: HIGH");
  }
  else
    INFO(__func__, "ENC_A: LOW");

  if( HIGH == digitalRead(ENC_PIN_B) )
  {
    INFO(__func__, "ENC_B: HIGH");
  }
  else
    INFO(__func__, "ENC_B: LOW");

  if( HIGH == digitalRead(ENC_PIN_C) )
  {
    INFO(__func__, "ENC_C: HIGH");
  }
  else
    INFO(__func__, "ENC_C: LOW");
}


/*
 *
 */
void beep()
{
  INFO(__func__, "beep!");
  beeper.start();
}


/*
 *
 */
void setEncoderInterrupt()
{
  int aNumber;  
  char *arg;

  INFO(__func__,""); 

  arg = SCmd.next(); 
  if(arg != NULL)
  {
    bInterruptEnabled = atoi(arg) ? true : false;
  }
  else
  {
    WARNING(__func__,"invalid arguments");
  }
}


 /*
 *
 */
void autoRead()
{
  readEOLsensors();
  readEncoders();
  delay(200);
  Serial.print(0x0C);
}


/*
 * This gets set as the default handler, and gets called when no other command matches. 
 */
void unrecognized()
{
  Serial.println("setSingleSolenoid [0..15] [1/0]");
  Serial.println("setSolenoids [1/0]");
  Serial.println("readEOLsensors");
  Serial.println("readEncoders");
  Serial.println("beep");
  Serial.println("setEncoderInterrupt");
  Serial.println("autoRead");
}
