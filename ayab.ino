// ayab.ino
/*
This file is part of AYAB.

    AYAB is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    AYAB is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with AYAB.  If not, see <http://www.gnu.org/licenses/>.

    Copyright 2013 Christian Obersteiner, Andreas Müller
    https://bitbucket.org/chris007de/ayab-apparat/
*/


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
 *  DECLARATIONS
 */ 
Knitter     *knitter;
byte        lineBuffer[25];

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
}


void loop() { 

  knitter->fsm();


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
   knitter->isr(); 
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
  //memset(lineBuffer,0,sizeof(lineBuffer));
  // temporary solution
  for( int i = 0; i < 25; i++)
  {
    lineBuffer[i] = 0xFF;
  }  

  bool _success = knitter->startOperation(_startNeedle, 
                                          _stopNeedle, 
                                          &(lineBuffer[0]));
  Serial.write(0xC1);
  Serial.write(_success);
  Serial.println("");
 }


 void h_cnfLine()
 {
  delay(50); //DEBUG wait for data to arrive
  byte _lineNumber = 0;
  byte _flags = 0;
  byte _crc8  = 0;
  bool _flagLastLine = false;

  _lineNumber = Serial.read();

  for( int i = 0; i < 25; i++ )
  { // Values have to be inverted because of needle states
    lineBuffer[i] = ~Serial.read();
  }
  _flags = Serial.read();
  _crc8  = Serial.read();

  // TODO insert CRC8 check

  if(knitter->setNextLine(_lineNumber))
  { // Line was accepted
    _flagLastLine = bitRead(_flags, 0);
    if( _flagLastLine )
    {
      knitter->setLastLine();
    }
  }
 }


 void h_reqInfo()
 {
  Serial.write(0xC3); //cnfInfo
  Serial.write(API_VERSION);
  Serial.println("");
 }


void h_unrecognized()
{
  return;
}