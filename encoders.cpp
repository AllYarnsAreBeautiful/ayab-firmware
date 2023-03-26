// encoders.cpp
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

    Copyright 2013-2015 Christian Obersteiner, Andreas Müller
    http://ayab-knitting.com
*/

#include "Arduino.h"
#include "./encoders.h"


Encoders::Encoders() {
  m_direction    = NoDirection;
  m_beltShift    = Unknown;
  m_carriage     = NoCarriage;
  m_encoderPos   = 0x00;

  m_leftHallSensor = new HallSensor(EOL_PIN_L);
  m_leftHallSensor->setThresholds(FILTER_L_MIN, FILTER_L_MAX);
  m_rightHallSensor = new HallSensor(EOL_PIN_R);
  m_rightHallSensor->setThresholds(FILTER_R_MIN, FILTER_R_MAX);
}

void Encoders::encA_interrupt() {
  static bool _old_encA = false;

  bool encA = digitalRead(ENC_PIN_A);
  bool encB = digitalRead(ENC_PIN_B);
  bool encC = digitalRead(ENC_PIN_C);

  if (!_old_encA && encA) { // Rising edge
    // Update direction
    m_direction = encB ? Right : Left;

    // Update carriage position & type
    if (m_direction == Right) {
      // Moving to the right, check left sensor
      if (m_leftHallSensor->isDetected(m_encoderPos, m_direction)) {
        m_encoderPos = END_LEFT + 28 + (m_encoderPos - m_leftHallSensor->position);
        m_carriage = m_leftHallSensor->carriage;
        // Belt phase for left sensor
        switch (m_carriage) {
          case L:
            m_beltShift = encC ? Regular : Shifted;
            break;
          default: // K & G
            m_beltShift = encC ? Shifted : Regular;
        }
      } else {
        if (m_encoderPos < END_RIGHT) {
          m_encoderPos++;
        }
      }
    }
  } else { // Falling edge
    // Update direction
    m_direction = encB ? Left : Right;
    
    // Update carriage position & type (commented out/HW NOK on KH910)
    if (m_direction == Left) {
//      // Moving to the left, check right sensor
//      if (m_rightHallSensor->isDetected(m_encoderPos, m_direction)) {
//        m_encoderPos = END_RIGHT - 28 - (m_rightHallSensor->position - m_encoderPos);
//        m_carriage = m_rightHallSensor->carriage;
//        // Belt phase for right sensor
//        m_beltShift = encC ? Regular : Shifted;
//      } else { 
        if (m_encoderPos > END_LEFT) {
          m_encoderPos--;
        }
//      }
    }
  }

  _old_encA = encA;

}

byte Encoders::getPosition() {
  return m_encoderPos;
}

Beltshift_t Encoders::getBeltshift() {
  return m_beltShift;
}

Direction_t Encoders::getDirection() {
  return m_direction;
}

Direction_t Encoders::getHallActive() {
  if (m_leftHallSensor->isActive()) {
    return Left;
  }
  if (m_rightHallSensor->isActive()) {
    return Right;
  }
  return NoDirection;
}

Carriage_t Encoders::getCarriage() {
  return m_carriage;
}

uint16 Encoders::getHallValue(Direction_t pSensor) {
  switch (pSensor) {
    case Left:
      return m_leftHallSensor->getValue();
    case Right:
      return m_rightHallSensor->getValue();
    default:
      return 0;
  }
}
