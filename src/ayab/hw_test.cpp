/*!
 * \file hw_test.cpp
 * This file is part of AYAB.
 *
 *    AYAB is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation, either version 3 of the License, or
 *    (at your option) any later version.
 *
 *    AYAB is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with AYAB.  If not, see <http://www.gnu.org/licenses/>.
 *
 *    Copyright 2013 Christian Obersteiner, Andreas MÃ¼ller
 *    http://ayab-knitting.com
 */

/* Includes */
#include <Arduino.h>

#include <SerialCommand.h>

#include "beeper.h"
#include "debug.h"
#include "settings.h"
#include "solenoids.h"

/* Global Declarations */
SerialCommand SCmd; ///< The SerialCommand object
Beeper beeper;
Solenoids solenoids;

/*!
 * \brief Beep command handler.
 */
void beep() {
  beeper.ready();
}

/*!
 * \brief Interrupt service routine for encoder A.
 */
void encoderAChange() {
  beep();
}

/*!
 * \brief Set single solenoid command handler.
 */
void setSingle() {
  int aNumber;
  char *arg;

  byte solenoidNumber = 0;
  byte solenoidState = 0;

  arg = SCmd.next();
  if (arg != NULL) {
    solenoidNumber = atoi(arg);
    arg = SCmd.next();
    if (arg != NULL) {
      solenoidState = atoi(arg);
      if (0 == solenoidState || 1 == solenoidState) {
        solenoids.setSolenoid(solenoidNumber, solenoidState);
      } else {
        DEBUG_PRINT("invalid arguments");
      }
    }
  }
}

/*!
 * \brief Set all solenoids command handler.
 */
void setAll() {
  int aNumber;
  char *arg;

  byte lowByte = 0;
  byte highByte = 0;
  uint16 solenoidState = 0;

  arg = SCmd.next();
  if (arg != NULL) {
    highByte = atoi(arg);

    arg = SCmd.next();
    if (arg != NULL) {
      lowByte = atoi(arg);
      solenoidState = (highByte << 8) + lowByte;
      solenoids.setSolenoids(solenoidState);
    }
  }
}

/*!
 * \brief Read EOL sensors command handler.
 */
void readEOLsensors() {
  Serial.print("  EOL_L: ");
  Serial.print(analogRead(EOL_PIN_L));
  Serial.print("  EOL_R: ");
  Serial.print(analogRead(EOL_PIN_R));
}

/*!
 * \brief Read encoders command handler.
 */
void readEncoders() {
  Serial.print("  ENC_A: ");
  if (HIGH == digitalRead(ENC_PIN_A)) {
    Serial.print("HIGH");
  } else {
    Serial.print("LOW");
  }

  Serial.print("  ENC_B: ");
  if (HIGH == digitalRead(ENC_PIN_B)) {
    Serial.print("HIGH");
  } else {
    Serial.print("LOW");
  }

  Serial.print("  ENC_C: ");
  if (HIGH == digitalRead(ENC_PIN_C)) {
    Serial.print("HIGH");
  } else {
    Serial.print("LOW");
  }
}

/*!
 * \brief Auto read command handler.
 */
void autoRead() {
  while (1) {
    readEOLsensors();
    readEncoders();
    delay(1000);
    Serial.println();
    // TODO fix clearscreen Serial.write(0x0C);
  }
}

/*!
 * \brief Auto test command handler.
 */
void autoTest() {
  while (1) {
    digitalWrite(LED_PIN_A, 1);
    digitalWrite(LED_PIN_B, 1);
    solenoids.setSolenoids(0xAAAA);
    delay(500);

    digitalWrite(LED_PIN_A, 0);
    digitalWrite(LED_PIN_B, 0);
    solenoids.setSolenoids(0x5555);
    delay(500);
  }
}

/*!
 * \brief Unrecognized command handler.
 *
 * This gets set as the default handler, and gets called when no other command
 * matches.
 */
void unrecognized() {
  Serial.println("setSingle [0..15] [1/0]");
  Serial.println("setAll [0..255] [0..255]");
  Serial.println("readEOLsensors");
  Serial.println("readEncoders");
  Serial.println("beep");
  Serial.println("autoRead");
  Serial.println("autoTest");
}

/*!
 * \brief Setup for hw tests.
 */
void hw_test_setup() {

  Serial.begin(SERIAL_BAUDRATE);

  pinMode(ENC_PIN_A, INPUT);
  pinMode(ENC_PIN_B, INPUT);
  pinMode(ENC_PIN_C, INPUT);

  pinMode(LED_PIN_A, OUTPUT);
  pinMode(LED_PIN_B, OUTPUT);

  // pinMode(PIEZO_PIN,OUTPUT);

  solenoids.init();

  // Setup callbacks for SerialCommand commands
  SCmd.addCommand("setSingle", setSingle);
  SCmd.addCommand("setAll", setAll);
  SCmd.addCommand("readEOLsensors", readEOLsensors);
  SCmd.addCommand("readEncoders", readEncoders);
  SCmd.addCommand("beep", beep);
  SCmd.addCommand("autoRead", autoRead);
  SCmd.addCommand("autoTest", autoTest);
  SCmd.setDefaultHandler(
      unrecognized); // Handler for command that isn't matched

  attachInterrupt(0, encoderAChange, RISING); // Attaching ENC_PIN_A(=2)

  // analogWrite(PIEZO_PIN, 100);
  DEBUG_PRINT("ready");
}

/*!
 * \brief Main loop for hw tests.
 */
void hw_test_loop() {
  SCmd.readSerial();
}
