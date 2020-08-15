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

// public interface

/*!
 * \brief Help command handler.
 */
void HardwareTest::helpCmd() {
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
}

/*!
 * \brief Send command handler.
 */
void HardwareTest::sendCmd() {
  Serial.println("Called send");
  uint8_t p[] = {1, 2, 3};
  knitter->send(p, 3);
  Serial.print("\n");
}

/*!
 * \brief Beep command handler.
 */
void HardwareTest::beepCmd() {
  Serial.println("Called beep");
  beep();
}

/*!
 * \brief Set single solenoid command handler.
 */
void HardwareTest::setSingleCmd() {
  Serial.println("Called setSingle");
  char *arg = m_sCmd.next();
  if (arg == nullptr) {
    return;
  }
  int solenoidNumber = atoi(arg);
  if (solenoidNumber < 0 or solenoidNumber > 15) {
    Serial.print("Invalid argument: ");
    Serial.println(solenoidNumber);
    return;
  }
  arg = m_sCmd.next();
  if (arg == nullptr) {
    return;
  }
  int solenoidState = atoi(arg);
  if (solenoidState < 0 or solenoidState > 1) {
    Serial.print("Invalid argument: ");
    Serial.println(solenoidState);
    return;
  }
  knitter->setSolenoid(static_cast<uint8_t>(solenoidNumber),
                       static_cast<uint8_t>(solenoidState));
}

/*!
 * \brief Set all solenoids command handler.
 */
void HardwareTest::setAllCmd() {
  Serial.println("Called setAll");
  char *arg = m_sCmd.next();
  if (arg == nullptr) {
    return;
  }
  uint8_t highByte = atoi(arg);
  arg = m_sCmd.next();
  if (arg == nullptr) {
    return;
  }
  uint8_t lowByte = atoi(arg);
  uint16_t solenoidState = (highByte << 8) + lowByte;
  knitter->setSolenoids(solenoidState);
}

/*!
 * \brief Read EOL sensors command handler.
 */
void HardwareTest::readEOLsensorsCmd() {
  Serial.println("Called readEOLsensors");
  readEOLsensors();
  Serial.print("\n");
}

/*!
 * \brief Read encoders command handler.
 */
void HardwareTest::readEncodersCmd() {
  Serial.println("Called readEncoders");
  readEncoders();
  Serial.print("\n");
}

/*!
 * \brief Auto read command handler.
 */
void HardwareTest::autoReadCmd() {
  Serial.println("Called autoRead, send stop to quit");
  m_autoReadOn = true;
}

/*!
 * \brief Auto test command handler.
 */
void HardwareTest::autoTestCmd() {
  Serial.println("Called autoTest, send stop to quit");
  m_autoTestOn = true;
}

/*!
 * \brief Stop command handler.
 */
void HardwareTest::stopCmd() {
  m_autoReadOn = false;
  m_autoTestOn = false;
}

/*!
 * \brief Quit command handler.
 */
void HardwareTest::quitCmd() {
  knitter->setQuitFlag(true);
  knitter->setUpInterrupt();
}

/*!
 * \brief Unrecognized command handler.
 *
 * \param buffer: pointer to string containing unrecognized command
 *
 * This gets set as the default handler, and gets called when no other command
 * matches.
 */
void HardwareTest::unrecognizedCmd(const char *buffer) {
  Serial.println("Unrecognized command");
  (void)(buffer); // does nothing but prevents 'unused variable' compile error
  helpCmd();
}

/*!
 * \brief Setup for hardware tests.
 */
void HardwareTest::setUp() {
  // set up callbacks for SerialCommand commands
  m_sCmd.addCommand("setSingle", GlobalHardwareTest::setSingleCmd);
  m_sCmd.addCommand("setAll", GlobalHardwareTest::setAllCmd);
  m_sCmd.addCommand("readEOLsensors", GlobalHardwareTest::readEOLsensorsCmd);
  m_sCmd.addCommand("readEncoders", GlobalHardwareTest::readEncodersCmd);
  m_sCmd.addCommand("beep", GlobalHardwareTest::beepCmd);
  m_sCmd.addCommand("autoRead", GlobalHardwareTest::autoReadCmd);
  m_sCmd.addCommand("autoTest", GlobalHardwareTest::autoTestCmd);
  m_sCmd.addCommand("send", GlobalHardwareTest::sendCmd);
  m_sCmd.addCommand("stop", GlobalHardwareTest::stopCmd);
  m_sCmd.addCommand("quit", GlobalHardwareTest::quitCmd);
  m_sCmd.addCommand("help", GlobalHardwareTest::helpCmd);
  m_sCmd.setDefaultHandler(GlobalHardwareTest::unrecognizedCmd);

  // Print welcome message
  Serial.print("AYAB Hardware Test, Firmware v");
  Serial.print(FW_VERSION_MAJ);
  Serial.print(".");
  Serial.print(FW_VERSION_MIN);
  Serial.print(" API v");
  Serial.print(API_VERSION);
  Serial.print("\n\n");
  helpCmd();

  // attach interrupt for ENC_PIN_A(=2), interrupt #0
  detachInterrupt(0);
#ifndef AYAB_TESTS
  attachInterrupt(0, GlobalHardwareTest::encoderAChange, RISING);
#endif // AYAB_TESTS

  m_lastTime = millis();
  m_autoReadOn = false;
  m_autoTestOn = false;
  m_timerEventOdd = false;
  knitter->setQuitFlag(false);
}

/*!
 * \brief Main loop for hardware tests.
 */
void HardwareTest::loop() {
  unsigned long now = millis();
  if (now - m_lastTime >= 500) {
    m_lastTime = now;
    handleTimerEvent();
  }
}

// Private member functions

void HardwareTest::beep() {
  knitter->m_beeper.ready();
}

void HardwareTest::readEncoders() {
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

void HardwareTest::readEOLsensors() {
  Serial.print("  EOL_L: ");
  Serial.print(analogRead(EOL_PIN_L));
  Serial.print("  EOL_R: ");
  Serial.print(analogRead(EOL_PIN_R));
}

void HardwareTest::autoRead() {
  Serial.print("\n");
  readEOLsensors();
  readEncoders();
  Serial.print("\n");
}

void HardwareTest::autoTestEven() {
  Serial.println("Set even solenoids");
  digitalWrite(LED_PIN_A, 1);
  digitalWrite(LED_PIN_B, 1);
  knitter->setSolenoids(0xAAAA);
}

void HardwareTest::autoTestOdd() {
  Serial.println("Set odd solenoids");
  digitalWrite(LED_PIN_A, 0);
  digitalWrite(LED_PIN_B, 0);
  knitter->setSolenoids(0x5555);
}

/*!
 * \brief Interrupt service routine for encoder A.
 */
#ifndef AYAB_TESTS
void HardwareTest::encoderAChange() {
  beep();
}
#endif // AYAB_TESTS

/*!
 * \brief Timer event every 500ms to handle auto functions.
 */
void HardwareTest::handleTimerEvent() {
  if (m_autoReadOn and m_timerEventOdd) {
    autoRead();
  }
  if (m_autoTestOn) {
    if (m_timerEventOdd) {
      autoTestOdd();
    } else {
      autoTestEven();
    }
  }
  m_timerEventOdd = not m_timerEventOdd;
  m_sCmd.readSerial();
}
