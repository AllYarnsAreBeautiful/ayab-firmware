/*!
 * \file tester.cpp
 * \brief Class containing methods for hardware testing.
 *
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
 *    Modified Work Copyright 2020-3 Sturla Lange, Tom Price
 *    http://ayab-knitting.com
 */

#include <Arduino.h>

#include "beeper.h"
#include "com.h"
#include "fsm.h"
#include "knitter.h"
#include "tester.h"

// public methods

/*!
 * \brief Help command handler.
 */
void Tester::helpCmd() {
  GlobalCom::sendMsg(testRes_msgid, "The following commands are available:\n");
  GlobalCom::sendMsg(testRes_msgid, "setSingle [0..15] [1/0]\n");
  GlobalCom::sendMsg(testRes_msgid, "setAll [0..FFFF]\n");
  GlobalCom::sendMsg(testRes_msgid, "readEOLsensors\n");
  GlobalCom::sendMsg(testRes_msgid, "readEncoders\n");
  GlobalCom::sendMsg(testRes_msgid, "beep\n");
  GlobalCom::sendMsg(testRes_msgid, "autoRead\n");
  GlobalCom::sendMsg(testRes_msgid, "autoTest\n");
  GlobalCom::sendMsg(testRes_msgid, "send\n");
  GlobalCom::sendMsg(testRes_msgid, "stop\n");
  GlobalCom::sendMsg(testRes_msgid, "quit\n");
  GlobalCom::sendMsg(testRes_msgid, "help\n");
}

/*!
 * \brief Send command handler.
 */
void Tester::sendCmd() {
  GlobalCom::sendMsg(testRes_msgid, "Called send\n");
  uint8_t p[] = {0x31, 0x32, 0x33};
  GlobalCom::send(p, 3);
  GlobalCom::sendMsg(testRes_msgid, "\n");
}

/*!
 * \brief Beep command handler.
 */
void Tester::beepCmd() {
  GlobalCom::sendMsg(testRes_msgid, "Called beep\n");
  beep();
}

/*!
 * \brief Set single solenoid command handler.
 * \param buffer Pointer to a data buffer.
 * \param size Number of bytes of data in the buffer.
 */
void Tester::setSingleCmd(const uint8_t *buffer, size_t size) {
  GlobalCom::sendMsg(testRes_msgid, "Called setSingle\n");
  if (size < 3U) {
    GlobalCom::sendMsg(testRes_msgid, "Error: invalid arguments\n");
    return;
  }
  uint8_t solenoidNumber = buffer[1];
  if (solenoidNumber > 15) {
    snprintf(buf, BUFFER_LEN, "Error: invalid solenoid index %i\n", solenoidNumber);
    GlobalCom::sendMsg(testRes_msgid, buf);
    return;
  }
  uint8_t solenoidState = buffer[2];
  if (solenoidState > 1) {
    snprintf(buf, BUFFER_LEN, "Error: invalid solenoid value %i\n", solenoidState);
    GlobalCom::sendMsg(testRes_msgid, buf);
    return;
  }
  GlobalSolenoids::setSolenoid(solenoidNumber, solenoidState);
}

/*!
 * \brief Set all solenoids command handler.
 * \param buffer Pointer to a data buffer.
 * \param size Number of bytes of data in the buffer.
 */
void Tester::setAllCmd(const uint8_t *buffer, size_t size) {
  GlobalCom::sendMsg(testRes_msgid, "Called setAll\n");
  if (size < 3U) {
    GlobalCom::sendMsg(testRes_msgid, "Error: invalid arguments\n");
    return;
  }
  uint16_t solenoidState = (buffer[1] << 8) + buffer[2];
  GlobalSolenoids::setSolenoids(solenoidState);
}

/*!
 * \brief Read EOL sensors command handler.
 */
void Tester::readEOLsensorsCmd() {
  GlobalCom::sendMsg(testRes_msgid, "Called readEOLsensors\n");
  readEOLsensors();
  GlobalCom::sendMsg(testRes_msgid, "\n");
}

/*!
 * \brief Read encoders command handler.
 */
void Tester::readEncodersCmd() {
  GlobalCom::sendMsg(testRes_msgid, "Called readEncoders\n");
  readEncoders();
  GlobalCom::sendMsg(testRes_msgid, "\n");
}

/*!
 * \brief Auto read command handler.
 */
void Tester::autoReadCmd() {
  GlobalCom::sendMsg(testRes_msgid, "Called autoRead, send stop to quit\n");
  m_autoReadOn = true;
}

/*!
 * \brief Auto test command handler.
 */
void Tester::autoTestCmd() {
  GlobalCom::sendMsg(testRes_msgid, "Called autoTest, send stop to quit\n");
  m_autoTestOn = true;
}

/*!
 * \brief Stop command handler.
 */
void Tester::stopCmd() {
  m_autoReadOn = false;
  m_autoTestOn = false;
}

/*!
 * \brief Quit command handler.
 */
void Tester::quitCmd() {
  GlobalFsm::setState(OpState::init);
  GlobalKnitter::setUpInterrupt();
}

/*!
 * \brief Start hardware test.
 * \param machineType Machine type.
 * \return Error code (0 = success, other values = error).
 */
Err_t Tester::startTest(Machine_t machineType) {
  OpState_t currentState = GlobalFsm::getState();
  if (OpState::wait_for_machine == currentState || OpState::init == currentState || OpState::ready == currentState) {
    GlobalFsm::setState(OpState::test);
    GlobalKnitter::setMachineType(machineType);
    setUp();
    return ErrorCode::SUCCESS;
  }
  return ErrorCode::ERR_WRONG_MACHINE_STATE;
}

/*!
 * \brief Main loop for hardware tests.
 */
void Tester::loop() {
  unsigned long now = millis();
  if (now - m_lastTime >= 500) {
    m_lastTime = now;
    handleTimerEvent();
  }
}

#ifndef AYAB_TESTS
/*!
 * \brief Interrupt service routine for encoder A.
 */
void Tester::encoderAChange() {
  beep();
}
#endif // AYAB_TESTS

// Private member functions

/*!
 * \brief Setup for hardware tests.
 */
void Tester::setUp() {
  // Print welcome message
  GlobalCom::sendMsg(testRes_msgid, "AYAB Hardware Test, ");
  snprintf(buf, BUFFER_LEN, "Firmware v%hhu", FW_VERSION_MAJ);
  GlobalCom::sendMsg(testRes_msgid, buf);
  snprintf(buf, BUFFER_LEN, ".%hhu", FW_VERSION_MIN);
  GlobalCom::sendMsg(testRes_msgid, buf);
  snprintf(buf, BUFFER_LEN, " API v%hhu\n\n", API_VERSION);
  GlobalCom::sendMsg(testRes_msgid, buf);
  helpCmd();

  // attach interrupt for ENC_PIN_A(=2), interrupt #0
  detachInterrupt(digitalPinToInterrupt(ENC_PIN_A));
#ifndef AYAB_TESTS
  // Attaching ENC_PIN_A, Interrupt #0
  // This interrupt cannot be enabled until
  // the machine type has been validated.
  attachInterrupt(digitalPinToInterrupt(ENC_PIN_A), GlobalTester::encoderAChange, RISING);
#endif // AYAB_TESTS

  m_autoReadOn = false;
  m_autoTestOn = false;
  m_lastTime = millis();
  m_timerEventOdd = false;
}

/*!
 * \brief Make a beep.
 */
void Tester::beep() const {
  GlobalBeeper::ready();
}

/*!
 * \brief Read the Hall sensors that determine which carriage is in use.
 */
void Tester::readEncoders() const {
  GlobalCom::sendMsg(testRes_msgid, "  ENC_A: ");
  bool state = digitalRead(ENC_PIN_A);
  GlobalCom::sendMsg(testRes_msgid, state ? "HIGH" : "LOW");
  GlobalCom::sendMsg(testRes_msgid, "  ENC_B: ");
  state = digitalRead(ENC_PIN_B);
  GlobalCom::sendMsg(testRes_msgid, state ? "HIGH" : "LOW");
  GlobalCom::sendMsg(testRes_msgid, "  ENC_C: ");
  state = digitalRead(ENC_PIN_C);
  GlobalCom::sendMsg(testRes_msgid, state ? "HIGH" : "LOW");
}

/*!
 * \brief Read the End of Line sensors.
 */
void Tester::readEOLsensors() {
  auto hallSensor = static_cast<uint16_t>(analogRead(EOL_PIN_L));
  snprintf(buf, BUFFER_LEN, "  EOL_L: %hu", hallSensor);
  GlobalCom::sendMsg(testRes_msgid, buf);
  hallSensor = static_cast<uint16_t>(analogRead(EOL_PIN_R));
  snprintf(buf, BUFFER_LEN, "  EOL_R: %hu", hallSensor);
  GlobalCom::sendMsg(testRes_msgid, buf);
}

/*!
 * \brief Read both carriage sensors and End of Line sensors.
 */
void Tester::autoRead() {
  GlobalCom::sendMsg(testRes_msgid, "\n");
  readEOLsensors();
  readEncoders();
  GlobalCom::sendMsg(testRes_msgid, "\n");
}

/*!
 * \brief Set even-numbered solenoids.
 */
void Tester::autoTestEven() const {
  GlobalCom::sendMsg(testRes_msgid, "Set even solenoids\n");
  digitalWrite(LED_PIN_A, HIGH);
  digitalWrite(LED_PIN_B, HIGH);
  GlobalSolenoids::setSolenoids(0xAAAA);
}

/*!
 * \brief Set odd-numbered solenoids.
 */
void Tester::autoTestOdd() const {
  GlobalCom::sendMsg(testRes_msgid, "Set odd solenoids\n");
  digitalWrite(LED_PIN_A, LOW);
  digitalWrite(LED_PIN_B, LOW);
  GlobalSolenoids::setSolenoids(0x5555);
}

/*!
 * \brief Timer event every 500ms to handle auto functions.
 */
void Tester::handleTimerEvent() {
  if (m_autoReadOn && m_timerEventOdd) {
    autoRead();
  }
  if (m_autoTestOn) {
    if (m_timerEventOdd) {
      autoTestOdd();
    } else {
      autoTestEven();
    }
  }
  m_timerEventOdd = !m_timerEventOdd;
}
