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
  
  //DEBUG_PRINT("#AYAB ready");
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
        h_cnfLine();
        break;

      case 0x03:  // reqInfo
        h_reqInfo();
        break;

      default:
        h_unrecognized();
        break;
    }
  }
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
  delay(50); //DEBUG wait for data to arrive
  byte _startNeedle = Serial.read();
  byte _stopNeedle  = Serial.read();
  
  // TODO verify operation
  memset(lineBuffer,0,sizeof(lineBuffer));

  bool _success = knitter->startOperation(_startNeedle, _stopNeedle);
  Serial.write(0xC1);
  Serial.write(_success);
  Serial.println("");
 }


 void h_cnfLine()
 {
  delay(50); //DEBUG wait for data to arrive
  static byte _currentBuffer = 0;
  byte _lineNumber = 0;
  byte _flags = 0;
  byte _crc8  = 0;
  bool _flagLastLine = false;

  _lineNumber = Serial.read();

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

  knitter->setNextLine(_lineNumber, &(lineBuffer[_currentBuffer][0]));

  _flagLastLine = bitRead(_flags, 0);
  if( _flagLastLine )
  {
    knitter->endWork();
  }
 }


 void h_reqInfo()
 {
 	Serial.write(0xC3); //cnfInfo
  Serial.write(VERSION);
 	Serial.println("");
 }


void h_unrecognized()
{
  return;
}
