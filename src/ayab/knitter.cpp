/*!
 * \file knitter.cpp
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
 *    Copyright 2013-2015 Christian Obersteiner, Andreas Müller
 *    http://ayab-knitting.com
 */

#include <Arduino.h>

#include "knitter.h"
#include "serial_encoding.h"

#ifndef AYAB_TESTS
/*!
 * \brief Wrapper for knitter's isr.
 *
 * This is needed since a non-static method cannot be
 * passed to _attachInterrupt_.
 */
void isr_wrapper() {
  extern Knitter *knitter;
  knitter->isr();
}
#endif

/*!
 * \brief Knitter constructor.
 *
 * Initializes the solenoids as well as pins and interrupts.
 */
Knitter::Knitter() {
  m_packetSerial.begin(SERIAL_BAUDRATE);
  m_packetSerial.setPacketHandler(&onPacketReceived);

  pinMode(ENC_PIN_A, INPUT);
#ifndef AYAB_TESTS
  // Attaching ENC_PIN_A(=2), Interrupt No. 0
  attachInterrupt(0, isr_wrapper, CHANGE);
#endif

  pinMode(ENC_PIN_B, INPUT);
  pinMode(ENC_PIN_C, INPUT);

  pinMode(LED_PIN_A, OUTPUT);
  pinMode(LED_PIN_B, OUTPUT);
  digitalWrite(LED_PIN_A, 1);
  digitalWrite(LED_PIN_B, 1);

#if DBG_NOMACHINE
  pinMode(DBG_BTN_PIN, INPUT);
#endif

  m_solenoids.init();
}

OpState_t Knitter::getState() {
  return m_opState;
}

void Knitter::send(uint8_t payload[], size_t length) {
  m_packetSerial.send(payload, length);
}

void Knitter::isr() {
  // Update machine state data
  m_encoders.encA_interrupt();
  m_position = m_encoders.getPosition();
  m_direction = m_encoders.getDirection();
  m_hallActive = m_encoders.getHallActive();
  m_beltshift = m_encoders.getBeltshift();
  m_carriage = m_encoders.getCarriage();
}

void Knitter::fsm() {
  switch (m_opState) {
  case s_init:
    state_init();
    break;

  case s_ready:
    state_ready();
    break;

  case s_operate:
    state_operate();
    break;

  case s_test:
    state_test();
    break;

  default:
    break;
  }
  m_packetSerial.update();
}

bool Knitter::startOperation(uint8_t startNeedle, uint8_t stopNeedle,
                             bool continuousReportingEnabled, uint8_t(*line)) {
  // TODO(sl): Check that functionality is correct after removing
  // always true comparison.
  if (stopNeedle < NUM_NEEDLES && startNeedle < stopNeedle) {
    if (s_ready == m_opState) {
      // Proceed to next state
      m_opState = s_operate;
      // Assign image width
      m_startNeedle = startNeedle;
      m_stopNeedle = stopNeedle;
      // Continuous Reporting enabled?
      m_continuousReportingEnabled = continuousReportingEnabled;
      // Set pixel data source
      m_lineBuffer = line;

      // Reset variables to start conditions
      m_currentLineNumber = 255; // because counter will
                                 // be increased before request
      m_lineRequested = false;
      m_lastLineFlag = false;
      m_lastLinesCountdown = 2;

      m_beeper.ready();

      return true;
    }
  }
  return false;
}

bool Knitter::startTest() {
  if (s_init == m_opState || s_ready == m_opState) {
    m_opState = s_test;
    return true;
  }
  return false;
}

bool Knitter::setNextLine(uint8_t lineNumber) {
  if (m_lineRequested) {
    // Is there even a need for a new line?
    if (lineNumber == m_currentLineNumber) {
      m_lineRequested = false;
      m_beeper.finishedLine();
      return true;
    } else {
      //  line numbers didnt match -> request again
      reqLine(m_currentLineNumber);
    }
  }
  return false;
}

void Knitter::setLastLine() {
  // lastLineFlag is evaluated in s_operate
  m_lastLineFlag = true;
}

/* Private Methods */

void Knitter::state_init() {
  // TODO(sl): Change into class members.
  static bool _ready = false;

#ifdef DBG_NOMACHINE
  // TODO(sl): Change into class members.
  static bool _prevState = false;
  bool state = digitalRead(DBG_BTN_PIN);

  // TODO Check if debounce is needed
  if (_prevState && !state) {
    _ready = true;
  }
  _prevState = state;
#else
  // Machine is initialized when left hall sensor is passed in Right direction
  if (Right == m_direction && Left == m_hallActive) {
    _ready = true;
  }
#endif // DBG_NOMACHINE

  if (_ready) {
    m_opState = s_ready;
    m_solenoids.setSolenoids(0xFFFF);
    indState(true);
    return;
  }

  m_opState = s_init;
}

void Knitter::state_ready() {
  digitalWrite(LED_PIN_A, 0);
  // This state is left when the startOperation() method
  // is called successfully by main()
}

void Knitter::state_operate() {
  digitalWrite(LED_PIN_A, 1);
  // TODO(sl): Change into class members.
  static bool _firstRun = true;
  static uint8_t _sOldPosition = 0;
  static bool _workedOnLine = false;

  if (true == _firstRun) {
    _firstRun = false;
    // Optimize Delay for various Arduino Models
    delay(2000);
    m_beeper.finishedLine();
    reqLine(++m_currentLineNumber);
  }

#ifdef DBG_NOMACHINE
  // TODO(sl): Change into class members.
  static bool _prevState = false;
  bool state = digitalRead(DBG_BTN_PIN);

  // TODO Check if debounce is needed
  if (_prevState && !state) {
    if (!m_lineRequested) {
      reqLine(++m_currentLineNumber);
    }
  }
  _prevState = state;
  return;
#else
  if (_sOldPosition != m_position) {
    // Only act if there is an actual change of position
    // Store current Encoder position for next call of this function
    _sOldPosition = m_position;

    if (m_continuousReportingEnabled) {
      // Send current position to GUI
      indState(true);
    }

    if (!calculatePixelAndSolenoid()) {
      // No valid/useful position calculated
      return;
    }

    if ((m_pixelToSet >= m_startNeedle - END_OF_LINE_OFFSET_L) &&
        (m_pixelToSet <= m_stopNeedle + END_OF_LINE_OFFSET_R)) {

      if ((m_pixelToSet >= m_startNeedle) && (m_pixelToSet <= m_stopNeedle)) {
        _workedOnLine = true;
      }

      // Find the right byte from the currentLine array,
      // then read the appropriate Pixel(/Bit) for the current needle to set
      int _currentByte = (int)(m_pixelToSet / 8);
      bool _pixelValue = bitRead(m_lineBuffer[_currentByte],
                                 m_pixelToSet - (8 * _currentByte));
      // Write Pixel state to the appropriate needle
      m_solenoids.setSolenoid(m_solenoidToSet, _pixelValue);
    } else { // Outside of the active needles
      //  digitalWrite(LED_PIN_B, 0);

      // Reset Solenoids when out of range
      m_solenoids.setSolenoid(m_solenoidToSet, true);

      if (_workedOnLine) {
        // already worked on the current line -> finished the line
        _workedOnLine = false;

        if (!m_lineRequested && !m_lastLineFlag) {
          // request new Line from Host
          reqLine(++m_currentLineNumber);
        } else if (m_lastLineFlag) {
          m_beeper.endWork();
          m_opState = s_ready;
          m_solenoids.setSolenoids(0xFFFF);
          m_beeper.finishedLine();
        }
      }
    }
  }
#endif // DBG_NOMACHINE
}

void Knitter::state_test() {
  // TODO(sl): Change into class members.
  static uint8_t _sOldPosition = 0;

  if (_sOldPosition != m_position) {
    // Only act if there is an actual change of position
    // Store current Encoder position for next call of this function
    _sOldPosition = m_position;

    calculatePixelAndSolenoid();
    indState();
  }
}

bool Knitter::calculatePixelAndSolenoid() {
  switch (m_direction) {
  // Calculate the solenoid and pixel to be set
  // Implemented according to machine manual
  // Magic numbers result from machine manual
  case Right:
    if (m_position >= getStartOffset(Left)) {
      m_pixelToSet = m_position - getStartOffset(Left);

      if (Regular == m_beltshift) {
        m_solenoidToSet = m_position % 16;
      } else if (Shifted == m_beltshift) {
        m_solenoidToSet = (m_position - 8) % 16;
      }

      if (L == m_carriage) {
        m_pixelToSet = m_pixelToSet + 8;
      }
    } else {
      return false;
    }
    break;

  case Left:
    if (m_position <= (END_RIGHT - getStartOffset(Right))) {
      m_pixelToSet = m_position - getStartOffset(Right);

      if (Regular == m_beltshift) {
        m_solenoidToSet = (m_position + 8) % 16;
      } else if (Shifted == m_beltshift) {
        m_solenoidToSet = m_position % 16;
      }

      if (L == m_carriage) {
        m_pixelToSet = m_pixelToSet - 16;
      }
    } else {
      return false;
    }
    break;

  default:
    return false;
    break;
  }
  return true;
}

uint8_t Knitter::getStartOffset(Direction_t direction) {
  switch (direction) {
  case Left:
    if (m_carriage == G) {
      // G carriage
      return 8;
    } else {
      // K and L carriage
      return 40;
    }
    break;

  case Right:
    if (m_carriage == G) {
      // G carriage
      return 32;
    } else {
      // K and L carriage
      return 16;
    }
    break;

  default:
    return 0;
  }
}

void Knitter::reqLine(uint8_t lineNumber) {
  uint8_t payload[2];
  payload[0] = reqLine_msgid;
  payload[1] = lineNumber;
  send(payload, 2);

  m_lineRequested = true;
}

void Knitter::indState(bool initState) {
  uint8_t payload[9];
  payload[0] = indState_msgid;
  payload[1] = (uint8_t)initState;

  uint16_t hallValue = m_encoders.getHallValue(Left);
  payload[2] = (uint8_t)(hallValue >> 8) & 0xFF;
  payload[3] = (uint8_t)hallValue & 0xFF;

  hallValue = m_encoders.getHallValue(Right);
  payload[4] = (uint8_t)(hallValue >> 8) & 0xFF;
  payload[5] = (uint8_t)hallValue & 0xFF;

  payload[6] = (uint8_t)m_carriage;
  payload[7] = (uint8_t)m_position;
  payload[8] = (uint8_t)m_encoders.getDirection();
  send(payload, 9);
}
