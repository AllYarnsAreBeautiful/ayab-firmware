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
 *    Original Work Copyright 2013 Christian Obersteiner, Andreas Müller
 *    Modified Work Copyright 2020 Sturla Lange, Tom Price
 *    http://ayab-knitting.com
 */

#include <Arduino.h>

#include "hw_test.h"
#include "knitter.h"

static void prompt() {
}

static void helpCmd() {
  Serial.println("The following commands are available:");
  Serial.println("setSingle [0..15] [1/0]");
  Serial.println("setAll [0..255] [0..255]");
  Serial.println("readEOLsensors");
  Serial.println("readEncoders");
  Serial.println("beep");
  Serial.println("autoRead");
  Serial.println("autoTest");
  Serial.println("send");
  Serial.println("stop");
  Serial.println("quit");
  Serial.println("help");

  prompt();
}

static void sendCmd() {
  extern Knitter *knitter;
  Serial.println("Called send");

  uint8_t p[] = {1, 2, 3};
  knitter->send(p, 3);
  Serial.print("\n");
  prompt();
}

static void beep() {
  hwTest->m_beeper.ready();
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
static void setSingleCmd() {
  Serial.println("Called setSingle");

  char *arg = hwTest->m_SCmd.next();
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
  arg = hwTest->m_SCmd.next();
  uint8_t solenoidState = atoi(arg);
  if (solenoidState > 1) {
    Serial.print("Invalid argument: ");
    Serial.println(solenoidState);
    return;
  }
  hwTest->m_solenoids.setSolenoid(solenoidNumber, solenoidState);
  prompt();
}

/*!
 * \brief Set all solenoids command handler.
 */
static void setAllCmd() {
  Serial.println("Called setAll");

  char *arg = hwTest->m_SCmd.next();
  if (arg == nullptr) {
    return;
  }
  uint8_t highByte = atoi(arg);
  arg = hwTest->m_SCmd.next();
  if (arg == nullptr) {
    return;
  }
  uint8_t lowByte = atoi(arg);
  uint16_t solenoidState = (highByte << 8) + lowByte;
  hwTest->m_solenoids.setSolenoids(solenoidState);
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
  // delay(1000);
}

/*!
 * \brief Auto read command handler.
 */
static void autoReadCmd() {
  Serial.println("Called autoRead, send stop to quit");
  hwTest->m_autoReadOn = true;
}

static void autoTestEven() {
  Serial.print("\n");
  Serial.println("Set even solenoids");
  digitalWrite(LED_PIN_A, 1);
  digitalWrite(LED_PIN_B, 1);
  hwTest->m_solenoids.setSolenoids(0xAAAA);
  // delay(500);
}

static void autoTestOdd() {
  Serial.println("Set odd solenoids");
  digitalWrite(LED_PIN_A, 0);
  digitalWrite(LED_PIN_B, 0);
  hwTest->m_solenoids.setSolenoids(0x5555);
  prompt();
  // delay(500);
}

/*!
 * \brief Auto test command handler.
 */
static void autoTestCmd() {
  Serial.println("Called autoTest, send stop to quit");
  hwTest->m_autoTestOn = true;
}

static void stopCmd() {
  hwTest->m_autoReadOn = false;
  hwTest->m_autoTestOn = false;
  prompt();
}

static void quitCmd() {
  extern Knitter *knitter;
  hwTest->m_quit = true;
  knitter->setUpInterrupt();
}

/*!
 * \brief Unrecognized command handler.
 *
 * \param buffer Unused buffer parameter.
 *
 * This gets set as the default handler, and gets called when no other command
 * matches.
 */
static void unrecognizedCmd(const char *buffer) {
  Serial.println("Unrecognized Command");
  (void)(buffer);
  helpCmd();
}

// Member functions

/*!
 * \brief Setup for hw tests.
 */
void HardwareTest::setUp() {
  // attach interrupt for ENC_PIN_A(=2), interrupt #0
  detachInterrupt(0);
  attachInterrupt(0, encoderAChange, RISING);

  // set up callbacks for SerialCommand commands
  m_SCmd.addCommand("setSingle", setSingleCmd);
  m_SCmd.addCommand("setAll", setAllCmd);
  m_SCmd.addCommand("readEOLsensors", readEOLsensorsCmd);
  m_SCmd.addCommand("readEncoders", readEncodersCmd);
  m_SCmd.addCommand("beep", beepCmd);
  m_SCmd.addCommand("autoRead", autoReadCmd);
  m_SCmd.addCommand("autoTest", autoTestCmd);
  m_SCmd.addCommand("send", sendCmd);
  m_SCmd.addCommand("stop", stopCmd);
  m_SCmd.addCommand("quit", quitCmd);
  m_SCmd.addCommand("help", helpCmd);
  m_SCmd.setDefaultHandler(unrecognizedCmd);

  // Print welcome message
  Serial.print("AYAB Hardware Test, Firmware v");
  Serial.print(FW_VERSION_MAJ);
  Serial.print(".");
  Serial.print(FW_VERSION_MIN);
  Serial.print(" API v");
  Serial.print(API_VERSION);
  Serial.print("\n\n");
  helpCmd();
}

/*!
 * \brief Main loop for hardware tests.
 */
void HardwareTest::loop() {
  if (m_autoReadOn and m_timerEvent and m_timerEventOdd) {
    autoRead();
  }
  if (m_autoTestOn) {
    if (m_timerEventOdd) {
      autoTestOdd();
    } else {
      autoTestEven();
    }
  }
  m_timerEvent = false;
  m_timerEventOdd = not m_timerEventOdd;
  m_SCmd.readSerial();
}
