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
byte        lineBuffer[NUM_LINE_BUFS][25];

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

  knitter = new Knitter();
  
  DEBUG_PRINT("#AYAB ready");
}


void loop() {   

  if( Serial.available() )
  {
    char inChar = (char)Serial.read();
    switch( inChar )
    {
      case 0x01:  // reqStart
        h_reqStart();
        break;

      case 0x42:  // cnfLine
        break;

      case 0x03:  // reqInfo
        h_reqInfo();
        break;

      default:
        h_unrecognized();
        break;
    }
  }

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


/*
 * Serial Command handling
 */
 void h_reqStart()
 {
  DEBUG_PRINT("");
  char _startNeedle = (char)Serial.read();
  char _stopNeedle  = (char)Serial.read();
  
  // TODO verify operation
  memset(lineBuffer,0,sizeof(lineBuffer));

  knitter->startOperation(_startNeedle, _stopNeedle);
 }


 void h_cnfLine()
 {
 	DEBUG_PRINT("");
  static _currentBuffer = 0;
  byte _flags = 0;
  byte _crc8  = 0;
  bool _flagLastLine = false;

  if( ++_currentBuffer >= NUM_LINE_BUFS)
  {
    _currentBuffer = 0;
  }
  
  for( int i = 0; i < 25; i++ )
  {
    lineBuffer[_currentBuffer][i] = Serial.read();
  }
  _flags = Serial.read();
  _crc8  = Serial.read();

  // TODO insert CRC8 check

  _flagLastLine = BitRead(_flags, 0);

  knitter->setNextLine(&lineBuffer[_currentBuffer][0], _flagLastLine);
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
