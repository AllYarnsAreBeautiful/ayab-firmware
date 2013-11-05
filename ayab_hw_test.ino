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
/*
 * SETUP
 */
void setup() {

	Serial.begin(9600);

  pinMode(ENC_A_PIN,INPUT);
  pinMode(ENC_B_PIN,INPUT);
  pinMode(ENC_C_PIN,INPUT);

	// Setup callbacks for SerialCommand commands
  SCmd.addCommand("setSingleSolenoid", setSingleSolenoid);
  SCmd.addCommand("setSolenoids", setSolenoids);
  SCmd.addCommand("readEOLsensors", readEOLsensors);
  SCmd.addCommand("readEncoders", readEncoders);
  SCmd.addCommand("beep", beep);
  SCmd.addCommand("autoRead", autoRead);
	SCmd.addDefaultHandler(unrecognized);  // Handler for command that isn't matched  (says "What?") 
	
  attachInterrupt(ENC_A_PIN, encoderAChange, CHANGE);

  INFO("setup", "ready");
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
  #define THIS "encoderAChange"

  INFO(THIS, "INTERRUPT");
}


/*
 *
 */
void setSingleSolenoid()    
{
  #define THIS "setSingleSolenoid"

  int aNumber;  
  char *arg;

  byte solenoidNumber = 0;
  byte solenoidState  = 0;

  INFO(THIS,""); 
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
        WARNING(THIS,"invalid arguments");
      }
    }
  }
}


/*
 *
 */
void setSolenoids()
{
  #define THIS "setSolenoids"

  int aNumber;  
  char *arg;

  byte solenoidState  = 0;

  INFO(THIS,""); 
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
      WARNING(THIS,"invalid arguments");
    }
  }
}


/*
 *
 */
void readEOLsensors()
{
  #define THIS "readEOLsensors"

  INFO(THIS, "EOL_R");
  INFO(THIS, atoi(analogRead(EOL_R_PIN)) );
  INFO(THIS, "EOL_L");
  INFO(THIS, atoi(analogRead(EOL_L_PIN)) );
}


/*
 *
 */
void readEncoders()
{
  #define THIS "readEncoders"

  if( HIGH == digitalRead(ENC_A_PIN) )
  {
    INFO(THIS, "ENC_A: HIGH");
  }
  else
    INFO(THIS, "ENC_A: LOW");

  if( HIGH == digitalRead(ENC_B_PIN) )
  {
    INFO(THIS, "ENC_B: HIGH");
  }
  else
    INFO(THIS, "ENC_B: LOW");

  if( HIGH == digitalRead(ENC_C_PIN) )
  {
    INFO(THIS, "ENC_C: HIGH");
  }
  else
    INFO(THIS, "ENC_C: LOW");
}


/*
 *
 */
void beep()
{
  #define THIS "beep"

  INFO(THIS, "beep!");
  beeper.start();
}


/*
 *
 */
void autoRead()
{
  #define THIS "autoRead"

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
  Serial.println("autoRead");
}