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

	Serial.begin(SERIAL_BAUDRATE);

  pinMode(ENC_PIN_A,INPUT);
  pinMode(ENC_PIN_B,INPUT);
  pinMode(ENC_PIN_C,INPUT);

  pinMode(LED_PIN_A,OUTPUT);
  pinMode(LED_PIN_B,OUTPUT); 

	// Setup callbacks for SerialCommand commands
  SCmd.addCommand("setSingle", setSingle);
  SCmd.addCommand("setAll", setAll);
  SCmd.addCommand("readEOLsensors", readEOLsensors);
  SCmd.addCommand("readEncoders", readEncoders);
  SCmd.addCommand("beep", beep);
  SCmd.addCommand("autoRead", autoRead);
	SCmd.addDefaultHandler(unrecognized);  // Handler for command that isn't matched 
	
  attachInterrupt(0, encoderAChange, RISING); //Attaching ENC_PIN_A(=2)

  DEBUG_PRINT("ready");
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
    beep();
}


/*
 *
 */
void setSingle()    
{
  int aNumber;  
  char *arg;

  byte solenoidNumber = 0;
  byte solenoidState  = 0;

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
        DEBUG_PRINT("invalid arguments");
      }
    }
  }
}


/*
 *
 */
void setAll()
{
  int aNumber;  
  char *arg;

  byte lowByte        = 0;
  byte highByte       = 0;
  uint16 solenoidState  = 0;
 
  arg = SCmd.next(); 
  if(arg != NULL)
  {
    highByte = atoi(arg);

    arg = SCmd.next(); 
    if(arg != NULL)
    {
      lowByte = atoi(arg);#
      solenoidState = (highByte << 8) + lowByte;
      solenoids.setSolenoids( solenoidState );    
    }    
  }
}


/*
 *
 */
void readEOLsensors()
{
  Serial.print("EOL_L: ");
  Serial.println(analogRead(EOL_PIN_L));
  Serial.print("EOL_R: ");
  Serial.println(analogRead(EOL_PIN_R));
}


/*
 *
 */
void readEncoders()
{
  if( HIGH == digitalRead(ENC_PIN_A) )
  {
    Serial.println("ENC_A: HIGH");
  }
  else
  {
    Serial.println("ENC_A: LOW");   
  }

  if( HIGH == digitalRead(ENC_PIN_B) )
  {
    Serial.println("ENC_B: HIGH");
  }
  else
  {
    Serial.println("ENC_B: LOW");
  }

  if( HIGH == digitalRead(ENC_PIN_C) )
  {
    Serial.println("ENC_C: HIGH");
  }
  else
  {
    Serial.println("ENC_C: LOW");
  }
}


/*
 *
 */
void beep()
{
  beeper.start();
}


 /*
 *
 */
void autoRead()
{
  while(1)
  {
    readEOLsensors();
    readEncoders();
    delay(1000);
    //TODO fix clearscreen Serial.write(0x0C);
  }
}


/*
 * This gets set as the default handler, and gets called when no other command matches. 
 */
void unrecognized()
{
  Serial.println("setSingle [0..15] [1/0]");
  Serial.println("setAll [0..255] [0..255]");
  Serial.println("readEOLsensors");
  Serial.println("readEncoders");
  Serial.println("beep");
  Serial.println("autoRead");
}