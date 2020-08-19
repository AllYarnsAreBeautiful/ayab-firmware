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

#include "global_hw_test.h"
#include "knitter.h"

// public interface

/*!
 * \brief Help command handler.
 */
void HardwareTest::helpCmd() {
  knitter->sendMsg(test_msgid, "The following commands are available:\n");
  knitter->sendMsg(test_msgid, "setSingle [0..15] [1/0]\n");
  knitter->sendMsg(test_msgid, "setAll [0..FFFF]\n");
  knitter->sendMsg(test_msgid, "readEOLsensors\n");
  knitter->sendMsg(test_msgid, "readEncoders\n");
  knitter->sendMsg(test_msgid, "beep\n");
  knitter->sendMsg(test_msgid, "autoRead\n");
  knitter->sendMsg(test_msgid, "autoTest\n");
  knitter->sendMsg(test_msgid, "send\n");
  knitter->sendMsg(test_msgid, "stop\n");
  knitter->sendMsg(test_msgid, "quit\n");
  knitter->sendMsg(test_msgid, "help\n");
}

/*!
 * \brief Send command handler.
 */
void HardwareTest::sendCmd() {
  knitter->sendMsg(test_msgid, "Called send\n");
  uint8_t p[] = {1, 2, 3};
  knitter->send(p, 3);
  knitter->sendMsg(test_msgid, "\n");
}

/*!
 * \brief Beep command handler.
 */
void HardwareTest::beepCmd() {
  knitter->sendMsg(test_msgid, "Called beep\n");
  beep();
}

/*!
 * \brief Set single solenoid command handler.
 */
void HardwareTest::setSingleCmd() {
  knitter->sendMsg(test_msgid, "Called setSingle\n");
  /*
  char *arg = m_sCmd.next();
  if (arg == nullptr) {
    return;
  }
  int solenoidNumber = atoi(arg);
  if (solenoidNumber < 0 or solenoidNumber > 15) {
    sprintf(buf, "Invalid argument: %i\n", solenoidNumber);
    knitter->sendMsg(test_msgid, buf);
    return;
  }
  arg = m_sCmd.next();
  if (arg == nullptr) {
    return;
  }
  int solenoidState = atoi(arg);
  if (solenoidState < 0 or solenoidState > 1) {
    sprintf(buf, "Invalid argument: %i\n", solenoidState);
    knitter->sendMsg(test_msgid, buf);
    return;
  }
  knitter->setSolenoid(static_cast<uint8_t>(solenoidNumber),
                       static_cast<uint8_t>(solenoidState));
  */
}

/*!
 * \brief Set all solenoids command handler.
 */
void HardwareTest::setAllCmd() {
  knitter->sendMsg(test_msgid, "Called setAll\n");
  /*
  char *arg = m_sCmd.next();
  if (arg == nullptr) {
    return;
  }
  short unsigned int solenoidState;
  // if (scanHex(arg, 4, &solenoidState)) {
  if (sscanf(arg, "%hx", &solenoidState)) {
    knitter->setSolenoids(solenoidState);
  } else {
    knitter->sendMsg(test_msgid, "Invalid argument. Please enter a hexadecimal "
                                 "number between 0 and FFFF.\n");
  }
  */
}

/*!  // GCOVR_EXCL_LINE
 * \brief Read EOL sensors command handler.
 */
void HardwareTest::readEOLsensorsCmd() {
  knitter->sendMsg(test_msgid, "Called readEOLsensors\n");
  readEOLsensors();
  knitter->sendMsg(test_msgid, "\n");
}

/*!  // GCOVR_EXCL_START
 * \brief Read encoders command handler.
 */  // GCOVR_EXCL_STOP
void HardwareTest::readEncodersCmd() {
  knitter->sendMsg(test_msgid, "Called readEncoders\n");
  readEncoders();
  knitter->sendMsg(test_msgid, "\n");
}

/*!  // GCOVR_EXCL_START
 * \brief Auto read command handler.
 */  // GCOVR_EXCL_STOP
void HardwareTest::autoReadCmd() {
  knitter->sendMsg(test_msgid, "Called autoRead, send stop to quit\n");
  m_autoReadOn = true;
}

/*!  // GCOVR_EXCL_START
 * \brief Auto test command handler.
 */  // GCOVR_EXCL_STOP
void HardwareTest::autoTestCmd() {
  knitter->sendMsg(test_msgid, "Called autoTest, send stop to quit\n");
  m_autoTestOn = true;
}

/*!  // GCOVR_EXCL_START
 * \brief Stop command handler.
 */  // GCOVR_EXCL_STOP
void HardwareTest::stopCmd() {
  m_autoReadOn = false;
  m_autoTestOn = false;
}

/*!  // GCOVR_EXCL_START
 * \brief Quit command handler.
 */  // GCOVR_EXCL_STOP
void HardwareTest::quitCmd() {
  knitter->setQuitFlag(true);
  knitter->setUpInterrupt();
}

/*!  // GCOVR_EXCL_START
 * \brief Unrecognized command handler.
 *
 * \param buffer: pointer to string containing unrecognized command
 *
 * This gets set as the default handler, and gets called when no other command
 * matches.
 */  // GCOVR_EXCL_STOP
void HardwareTest::unrecognizedCmd(const char *buffer) {
  knitter->sendMsg(test_msgid, "Unrecognized command\n");
  (void)(buffer); // does nothing but prevents 'unused variable' compile error
  helpCmd();
}

/*!  // GCOVR_EXCL_START
 * \brief Setup for hardware tests.
 */  // GCOVR_EXCL_STOP
void HardwareTest::setUp() {
  // set up callbacks for SerialCommand commands
  /*
  m_sCmd.addCommand("%setSingle", GlobalHardwareTest::setSingleCmd);
  m_sCmd.addCommand("%setAll", GlobalHardwareTest::setAllCmd);
  m_sCmd.addCommand("%readEOLsensors", GlobalHardwareTest::readEOLsensorsCmd);
  m_sCmd.addCommand("%readEncoders", GlobalHardwareTest::readEncodersCmd);
  m_sCmd.addCommand("%beep", GlobalHardwareTest::beepCmd);
  m_sCmd.addCommand("%autoRead", GlobalHardwareTest::autoReadCmd);
  m_sCmd.addCommand("%autoTest", GlobalHardwareTest::autoTestCmd);
  m_sCmd.addCommand("%send", GlobalHardwareTest::sendCmd);
  m_sCmd.addCommand("%stop", GlobalHardwareTest::stopCmd);
  m_sCmd.addCommand("%quit", GlobalHardwareTest::quitCmd);
  m_sCmd.addCommand("%help", GlobalHardwareTest::helpCmd);
  m_sCmd.setDefaultHandler(GlobalHardwareTest::unrecognizedCmd);
  */

  // Print welcome message
  knitter->sendMsg(test_msgid, "AYAB Hardware Test, ");
  sprintf(buf, "Firmware v%hhu", FW_VERSION_MAJ);
  knitter->sendMsg(test_msgid, buf);
  sprintf(buf, ".%hhu", FW_VERSION_MIN);
  knitter->sendMsg(test_msgid, buf);
  sprintf(buf, " API v%hhu\n\n", API_VERSION);
  knitter->sendMsg(test_msgid, buf);
  helpCmd();

  // attach interrupt for ENC_PIN_A(=2), interrupt #0
  detachInterrupt(0);
#ifndef AYAB_TESTS
  attachInterrupt(0, GlobalHardwareTest::encoderAChange, RISING);
#endif // AYAB_TESTS  // GCOVR_EXCL_LINE

  m_lastTime = millis();
  m_autoReadOn = false;
  m_autoTestOn = false;
  m_timerEventOdd = false;
  knitter->setQuitFlag(false);
}

/*!  // GCOVR_EXCL_START
 * \brief Main loop for hardware tests.
 */  // GCOVR_EXCL_STOP
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
  knitter->sendMsg(test_msgid, "  ENC_A: ");
  bool state = digitalRead(ENC_PIN_A);
  knitter->sendMsg(test_msgid, state ? "HIGH" : "LOW");
  knitter->sendMsg(test_msgid, "  ENC_B: ");
  state = digitalRead(ENC_PIN_B);
  knitter->sendMsg(test_msgid, state ? "HIGH" : "LOW");
  knitter->sendMsg(test_msgid, "  ENC_C: ");
  state = digitalRead(ENC_PIN_C);
  knitter->sendMsg(test_msgid, state ? "HIGH" : "LOW");
}

void HardwareTest::readEOLsensors() { // GCOVR_EXCL_LINE (?)
  uint16_t hallSensor = static_cast<uint16_t>(analogRead(EOL_PIN_L));
  sprintf(buf, "  EOL_L: %hu", hallSensor);
  knitter->sendMsg(test_msgid, buf);
  hallSensor = static_cast<uint16_t>(analogRead(EOL_PIN_R));
  sprintf(buf, "  EOL_R: %hu", hallSensor);
  knitter->sendMsg(test_msgid, buf);
}

void HardwareTest::autoRead() {
  knitter->sendMsg(test_msgid, "\n");
  readEOLsensors();
  readEncoders();
  knitter->sendMsg(test_msgid, "\n");
}

void HardwareTest::autoTestEven() {
  knitter->sendMsg(test_msgid, "Set even solenoids\n");
  digitalWrite(LED_PIN_A, HIGH);
  digitalWrite(LED_PIN_B, HIGH);
  knitter->setSolenoids(0xAAAA);
}

void HardwareTest::autoTestOdd() {
  knitter->sendMsg(test_msgid, "Set odd solenoids\n");
  digitalWrite(LED_PIN_A, LOW);
  digitalWrite(LED_PIN_B, LOW);
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
  /*
  m_sCmd.readSerial();
  */
}

/*
// homebrew `sscanf(str, "%hx", &result);`
// does not trim white space
bool HardwareTest::scanHex(char *str, uint8_t maxDigits, uint16_t *result) {
  if (maxDigits == 0 or *str == 0) {
    return false;
  }
  uint16_t s = 0;
  char c;
  while (maxDigits-- > 0 and (c = *str++) != 0) {
    s <<= 4;
    if ('0' <= c and c <= '9') {
      s += c - '0';
    } else if ('a' <= c and c <= 'f') {
      s += c + 10 - 'a';
    } else if ('A' <= c and c <= 'F') {
      s += c + 10 - 'A';
    } else {
      return false;
    }
  }
  *result = s;
  return true;
}
*/
