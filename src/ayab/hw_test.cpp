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
#include <Arduino.h>

#include <SerialCommand.h>

#include "knitter.h"

extern Knitter *knitter;

static SerialCommand SCmd; ///< The SerialCommand object
static Beeper beeper;
static Solenoids solenoids;

static bool autoReadOn = false;
static bool autoTestOn = false;

static void prompt() {
  Serial.print("$ ");
}

static void help() {
  Serial.println("The following commands are available:");
  Serial.println("setSingle [0..15] [1/0]");
  Serial.println("setAll [0..255] [0..255]");
  Serial.println("readEOLsensors");
  Serial.println("readEncoders");
  Serial.println("beep");
  Serial.println("autoRead");
  Serial.println("autoTest");
  Serial.println("send");
  Serial.println("help");

  prompt();
}

static void send() {
  Serial.println("Called send");

  uint8_t p[] = {1, 2, 3};
  knitter->send(p, 3);
  Serial.print("\n");

  prompt();
}

static void beep() {
  beeper.ready();
}

/*!
 * \brief Beep command handler.
 */
static void beepCmd() {
  Serial.println("Called beep");

  beep();

  prompt();
}

/*!
 * \brief Interrupt service routine for encoder A.
 */
static void encoderAChange() {
  beep();
}

/*!
 * \brief Set single solenoid command handler.
 */
static void setSingle() {
  Serial.println("Called setSingle");

  char *arg = SCmd.next();
  if (arg == nullptr) {
    return;
  }
  uint8_t solenoidNumber = atoi(arg);
  if (solenoidNumber > 15) {
    Serial.print("Invalid argument: ");
    Serial.println(solenoidNumber);
    return;
  }

  if (arg == nullptr) {
    return;
  }
  arg = SCmd.next();
  uint8_t solenoidState = atoi(arg);
  if (solenoidState > 1) {
    Serial.print("Invalid argument: ");
    Serial.println(solenoidState);
    return;
  }

  solenoids.setSolenoid(solenoidNumber, solenoidState);

  prompt();
}

/*!
 * \brief Set all solenoids command handler.
 */
static void setAll() {
  Serial.println("Called setAll");

  char *arg = SCmd.next();
  if (arg == nullptr) {
    return;
  }
  uint8_t highByte = atoi(arg);

  arg = SCmd.next();
  if (arg == nullptr) {
    return;
  }
  uint8_t lowByte = atoi(arg);

  uint16_t solenoidState = (highByte << 8) + lowByte;
  solenoids.setSolenoids(solenoidState);

  prompt();
}

static void readEOLsensors() {
  Serial.print("  EOL_L: ");
  Serial.print(analogRead(EOL_PIN_L));

  Serial.print("  EOL_R: ");
  Serial.print(analogRead(EOL_PIN_R));
}

/*!
 * \brief Read EOL sensors command handler.
 */
static void readEOLsensorsCmd() {
  Serial.println("Called readEOLsensors");

  readEOLsensors();
  Serial.print("\n");

  prompt();
}

static void readEncoders() {
  Serial.print("  ENC_A: ");
  bool state = digitalRead(ENC_PIN_A);
  Serial.print(state ? "HIGH" : "LOW");

  Serial.print("  ENC_B: ");
  state = digitalRead(ENC_PIN_B);
  Serial.print(state ? "HIGH" : "LOW");

  Serial.print("  ENC_C: ");
  state = digitalRead(ENC_PIN_C);
  Serial.print(state ? "HIGH" : "LOW");
}

/*!
 * \brief Read encoders command handler.
 */
static void readEncodersCmd() {
  Serial.println("Called readEncoders");

  readEncoders();
  Serial.print("\n");

  prompt();
}

static void autoRead() {
  Serial.print("\n");
  readEOLsensors();
  readEncoders();
  Serial.print("\n");

  prompt();
  delay(1000);
}

/*!
 * \brief Auto read command handler.
 */
static void autoReadCmd() {
  Serial.println("Called autoRead, send stop to quit");
  autoReadOn = true;
}

static void autoTest() {
  Serial.print("\n");

  Serial.println("Set even solenoids");
  digitalWrite(LED_PIN_A, 1);
  digitalWrite(LED_PIN_B, 1);
  solenoids.setSolenoids(0xAAAA);

  delay(500);

  Serial.println("Set odd solenoids");
  digitalWrite(LED_PIN_A, 0);
  digitalWrite(LED_PIN_B, 0);
  solenoids.setSolenoids(0x5555);

  prompt();
  delay(500);
}

/*!
 * \brief Auto test command handler.
 */
static void autoTestCmd() {
  Serial.println("Called autoTest, send stop to quit");
  autoTestOn = true;
}

static void stop() {
  autoReadOn = false;
  autoTestOn = false;

  prompt();
}

/*!
 * \brief Unrecognized command handler.
 *
 * \param buffer Unused buffer parameter.
 *
 * This gets set as the default handler, and gets called when no other command
 * matches.
 */
static void unrecognized(const char *buffer) {
  Serial.println("Unrecognized Command");

  (void)(buffer);

  help();
}

/*!
 * \brief Setup for hw tests.
 */
void hw_test_setup() {
  pinMode(LED_PIN_A, OUTPUT);
  pinMode(LED_PIN_B, OUTPUT);

  // Setup callbacks for SerialCommand commands
  SCmd.addCommand("setSingle", setSingle);
  SCmd.addCommand("setAll", setAll);
  SCmd.addCommand("readEOLsensors", readEOLsensorsCmd);
  SCmd.addCommand("readEncoders", readEncodersCmd);
  SCmd.addCommand("beep", beepCmd);
  SCmd.addCommand("autoRead", autoReadCmd);
  SCmd.addCommand("autoTest", autoTestCmd);
  SCmd.addCommand("send", send);
  SCmd.addCommand("stop", stop);
  SCmd.addCommand("help", help);
  SCmd.setDefaultHandler(
      unrecognized); // Handler for command that isn't matched

  attachInterrupt(0, encoderAChange, RISING); // Attaching ENC_PIN_A(=2)

  Serial.print("AYAB HW Test Firmware v");
  Serial.print(FW_VERSION_MAJ);
  Serial.print(".");
  Serial.print(FW_VERSION_MIN);
  Serial.print(" API v");
  Serial.print(API_VERSION);
  Serial.print("\n\n");

  help();
}

/*!
 * \brief Main loop for hw tests.
 */
void hw_test_loop() {
  if (autoReadOn) {
    autoRead();
  }
  if (autoTestOn) {
    autoTest();
  }
  SCmd.readSerial();
}
