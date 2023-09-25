/*!
 * \file opTest.cpp
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

#include "board.h"

#include "beeper.h"
#include "com.h"
#include "controller.h"
#include "encoders.h"
#include "solenoids.h"

#include "opInit.h"
#include "opKnit.h"
#include "opTest.h"

// public methods

/*!
 * \brief enum OpState
 * \return OpState_t::Test
 */
OpState_t OpTest::state() {
  return OpState_t::Test;
}

/*!
 * \brief Initialization for hardware tests.
 */
void OpTest::init() {
}

/*!
 * \brief Start hardware test.
 * \param machineType Machine type.
 */
void OpTest::begin() {
  // Print welcome message
  GlobalCom::sendMsg(API_t::testRes, "AYAB Hardware OpTest, ");
  snprintf(buf, BUFFER_LEN, "Firmware v%hhu", FW_VERSION_MAJ);
  GlobalCom::sendMsg(API_t::testRes, buf);
  snprintf(buf, BUFFER_LEN, ".%hhu", FW_VERSION_MIN);
  GlobalCom::sendMsg(API_t::testRes, buf);
  snprintf(buf, BUFFER_LEN, " API v%hhu\n\n", API_VERSION);
  GlobalCom::sendMsg(API_t::testRes, buf);
  helpCmd();

  // attach interrupt for ENC_PIN_A(=2), interrupt #0
  detachInterrupt(digitalPinToInterrupt(ENC_PIN_A));
#ifndef AYAB_TESTS
  // Attaching ENC_PIN_A, Interrupt #0
  // This interrupt cannot be enabled until
  // the machine type has been validated.
  attachInterrupt(digitalPinToInterrupt(ENC_PIN_A), GlobalOpTest::encoderAChange, RISING);
#endif // AYAB_TESTS

  m_autoReadOn = false;
  m_autoTestOn = false;
  m_lastTime = millis();
  m_timerEventOdd = false;
}

/*!
 * \brief Main loop for hardware tests.
 */
void OpTest::update() {
  if (enabled()) {
    GlobalOpKnit::encodePosition();
    uint32_t now = millis();
    if (now - m_lastTime >= TEST_LOOP_DELAY) {
      m_lastTime = now;
      handleTimerEvent();
    }
  }
}

/*!
 * \brief Communication callback for hardware tests.
 */
void OpTest::com(const uint8_t *buffer, size_t size) {
  switch (buffer[0]) {
  case static_cast<uint8_t>(API_t::helpCmd):
    helpCmd();
    break;

  case static_cast<uint8_t>(API_t::sendCmd):
    sendCmd();
    break;

  case static_cast<uint8_t>(API_t::beepCmd):
    beepCmd();
    break;

  case static_cast<uint8_t>(API_t::setSingleCmd):
    setSingleCmd(buffer, size);
    break;

  case static_cast<uint8_t>(API_t::setAllCmd):
    setAllCmd(buffer, size);
    break;

  case static_cast<uint8_t>(API_t::readEOLsensorsCmd):
    readEOLsensorsCmd();
    break;

  case static_cast<uint8_t>(API_t::readEncodersCmd):
    readEncodersCmd();
    break;

  case static_cast<uint8_t>(API_t::autoReadCmd):
    autoReadCmd();
    break;

  case static_cast<uint8_t>(API_t::autoTestCmd):
    autoTestCmd();
    break;

  case static_cast<uint8_t>(API_t::stopCmd):
    stopCmd();
    break;

  case static_cast<uint8_t>(API_t::quitCmd):
    end();
    break;

  default:
    GlobalCom::h_unrecognized();
    break;
  }
}

/*!
 * \brief Finish hardware tests.
 */
void OpTest::end() {
  m_autoReadOn = false;
  m_autoTestOn = false;
  GlobalController::setState(GlobalOpInit::m_instance);
  GlobalOpKnit::init();
  GlobalEncoders::setUpInterrupt();
}

/*!
 * \brief Returns whether the hardware test loop is active.
 */
 bool OpTest::enabled() {
  return m_autoReadOn || m_autoTestOn;
}

/*!
 * \brief Help command handler.
 */
void OpTest::helpCmd() {
  GlobalCom::sendMsg(API_t::testRes, "The following commands are available:\n");
  GlobalCom::sendMsg(API_t::testRes, "setSingle [0..15] [1/0]\n");
  GlobalCom::sendMsg(API_t::testRes, "setAll [0..FFFF]\n");
  GlobalCom::sendMsg(API_t::testRes, "readEOLsensors\n");
  GlobalCom::sendMsg(API_t::testRes, "readEncoders\n");
  GlobalCom::sendMsg(API_t::testRes, "beep\n");
  GlobalCom::sendMsg(API_t::testRes, "autoRead\n");
  GlobalCom::sendMsg(API_t::testRes, "autoTest\n");
  GlobalCom::sendMsg(API_t::testRes, "send\n");
  GlobalCom::sendMsg(API_t::testRes, "stop\n");
  GlobalCom::sendMsg(API_t::testRes, "quit\n");
  GlobalCom::sendMsg(API_t::testRes, "help\n");
}

/*!
 * \brief Send command handler.
 */
void OpTest::sendCmd() {
  GlobalCom::sendMsg(API_t::testRes, "Called send\n");
  uint8_t p[] = {0x31, 0x32, 0x33};
  GlobalCom::send(p, 3);
  GlobalCom::sendMsg(API_t::testRes, "\n");
}

/*!
 * \brief Beep command handler.
 */
void OpTest::beepCmd() {
  GlobalCom::sendMsg(API_t::testRes, "Called beep\n");
  beep();
}

/*!
 * \brief Set single solenoid command handler.
 * \param buffer Pointer to a data buffer.
 * \param size Number of bytes of data in the buffer.
 */
void OpTest::setSingleCmd(const uint8_t *buffer, size_t size) {
  GlobalCom::sendMsg(API_t::testRes, "Called setSingle\n");
  if (size < 3U) {
    GlobalCom::sendMsg(API_t::testRes, "Error: invalid arguments\n");
    return;
  }
  uint8_t solenoidNumber = buffer[1];
  if (solenoidNumber > 15) {
    snprintf(buf, BUFFER_LEN, "Error: invalid solenoid index %i\n", solenoidNumber);
    GlobalCom::sendMsg(API_t::testRes, buf);
    return;
  }
  uint8_t solenoidState = buffer[2];
  if (solenoidState > 1) {
    snprintf(buf, BUFFER_LEN, "Error: invalid solenoid value %i\n", solenoidState);
    GlobalCom::sendMsg(API_t::testRes, buf);
    return;
  }
  GlobalSolenoids::setSolenoid(solenoidNumber, solenoidState);
}

/*!
 * \brief Set all solenoids command handler.
 * \param buffer Pointer to a data buffer.
 * \param size Number of bytes of data in the buffer.
 */
void OpTest::setAllCmd(const uint8_t *buffer, size_t size) {
  GlobalCom::sendMsg(API_t::testRes, "Called setAll\n");
  if (size < 3U) {
    GlobalCom::sendMsg(API_t::testRes, "Error: invalid arguments\n");
    return;
  }
  uint16_t solenoidState = (buffer[1] << 8) + buffer[2];
  GlobalSolenoids::setSolenoids(solenoidState);
}

/*!
 * \brief Read EOL sensors command handler.
 */
void OpTest::readEOLsensorsCmd() {
  GlobalCom::sendMsg(API_t::testRes, "Called readEOLsensors\n");
  readEOLsensors();
  GlobalCom::sendMsg(API_t::testRes, "\n");
}

/*!
 * \brief Read encoders command handler.
 */
void OpTest::readEncodersCmd() {
  GlobalCom::sendMsg(API_t::testRes, "Called readEncoders\n");
  readEncoders();
  GlobalCom::sendMsg(API_t::testRes, "\n");
}

/*!
 * \brief Auto read command handler.
 */
void OpTest::autoReadCmd() {
  GlobalCom::sendMsg(API_t::testRes, "Called autoRead, send stop to quit\n");
  m_autoReadOn = true;
}

/*!
 * \brief Auto test command handler.
 */
void OpTest::autoTestCmd() {
  GlobalCom::sendMsg(API_t::testRes, "Called autoTest, send stop to quit\n");
  m_autoTestOn = true;
}

/*!
 * \brief Stop command handler.
 */
void OpTest::stopCmd() {
  m_autoReadOn = false;
  m_autoTestOn = false;
}

#ifndef AYAB_TESTS
/*!
 * \brief Interrupt service routine for encoder A.
 */
void OpTest::encoderAChange() {
  beep();
}
#endif // AYAB_TESTS

// Private member functions

/*!
 * \brief Make a beep.
 */
void OpTest::beep() const {
  GlobalBeeper::ready();
}

/*!
 * \brief Read the Hall sensors that determine which carriage is in use.
 */
void OpTest::readEncoders() const {
  GlobalCom::sendMsg(API_t::testRes, "  ENC_A: ");
  bool state = digitalRead(ENC_PIN_A);
  GlobalCom::sendMsg(API_t::testRes, state ? "HIGH" : "LOW");
  GlobalCom::sendMsg(API_t::testRes, "  ENC_B: ");
  state = digitalRead(ENC_PIN_B);
  GlobalCom::sendMsg(API_t::testRes, state ? "HIGH" : "LOW");
  GlobalCom::sendMsg(API_t::testRes, "  ENC_C: ");
  state = digitalRead(ENC_PIN_C);
  GlobalCom::sendMsg(API_t::testRes, state ? "HIGH" : "LOW");
}

/*!
 * \brief Read the End of Line sensors.
 */
void OpTest::readEOLsensors() {
  auto hallSensor = static_cast<uint16_t>(analogRead(EOL_PIN_L));
  snprintf(buf, BUFFER_LEN, "  EOL_L: %hu", hallSensor);
  GlobalCom::sendMsg(API_t::testRes, buf);
  hallSensor = static_cast<uint16_t>(analogRead(EOL_PIN_R));
  snprintf(buf, BUFFER_LEN, "  EOL_R: %hu", hallSensor);
  GlobalCom::sendMsg(API_t::testRes, buf);
}

/*!
 * \brief Read both carriage sensors and End of Line sensors.
 */
void OpTest::autoRead() {
  GlobalCom::sendMsg(API_t::testRes, "\n");
  readEOLsensors();
  readEncoders();
  GlobalCom::sendMsg(API_t::testRes, "\n");
}

/*!
 * \brief Set even-numbered solenoids.
 */
void OpTest::autoTestEven() const {
  GlobalCom::sendMsg(API_t::testRes, "Set even solenoids\n");
  digitalWrite(LED_PIN_A, HIGH);
  digitalWrite(LED_PIN_B, HIGH);
  GlobalSolenoids::setSolenoids(0xAAAA);
}

/*!
 * \brief Set odd-numbered solenoids.
 */
void OpTest::autoTestOdd() const {
  GlobalCom::sendMsg(API_t::testRes, "Set odd solenoids\n");
  digitalWrite(LED_PIN_A, LOW);
  digitalWrite(LED_PIN_B, LOW);
  GlobalSolenoids::setSolenoids(0x5555);
}

/*!
 * \brief Timer event every 500ms to handle auto functions.
 */
void OpTest::handleTimerEvent() {
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
