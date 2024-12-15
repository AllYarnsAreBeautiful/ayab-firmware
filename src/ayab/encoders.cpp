/*!
 * \file encoders.cpp
 * \brief Class containing methods governing encoders
 *    for BeltShift, Direction, Active Hall sensor and Carriage Type.
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
 *    Original Work Copyright 2013-2015 Christian Obersteiner, Andreas Müller
 *    Modified Work Copyright 2020 Sturla Lange, Tom Price
 *    http://ayab-knitting.com
 */

#include "board.h"
#include <Arduino.h>

#include "encoders.h"


/*!
 * \brief Service encoder A interrupt routine.
 *
 * Determines edge of signal and dispatches to private rising/falling functions.
 * `m_machineType` assumed valid.
 */
void Encoders::encA_interrupt() {
  m_hallActive = Direction_t::NoDirection;

  auto currentState = static_cast<bool>(digitalRead(ENC_PIN_A));

  if (!m_oldState && currentState) {
    encA_rising();
  } else if (m_oldState && !currentState) {
    encA_falling();
  }
  m_oldState = currentState;
}

/*!
 * \brief Read hall sensor on left and right.
 * \param pSensor Which sensor to read (left or right).
 */
uint16_t Encoders::getHallValue(Direction_t pSensor) {
  switch (pSensor) {
  case Direction_t::Left:
    return analogRead(EOL_PIN_L);
  case Direction_t::Right:
    return analogRead(EOL_PIN_R);
  default:
    return 0;
  }
}

/*!
 * \brief Initialize machine type.
 * \param machineType Machine type.
 */
void Encoders::init(Machine_t machineType) {
  m_machineType = machineType;
  m_position = 0U;
  m_direction = Direction_t::NoDirection;
  m_hallActive = Direction_t::NoDirection;
  m_beltShift = BeltShift::Unknown;
  m_carriage = Carriage_t::NoCarriage;
  m_previousDetectedCarriageLeft = detectCarriageLeft();
  m_oldState = false;
  m_passedLeft = false;
  m_passedRight = false;
}

/*!
 * \brief Get position member.
 */
uint8_t Encoders::getPosition() {
  return m_position;
}

/*!
 * \brief Get beltShift member.
 */
BeltShift_t Encoders::getBeltShift() {
  return m_beltShift;
}

/*!
 * \brief Get direction member.
 */
Direction_t Encoders::getDirection() {
  return m_direction;
}

/*!
 * \brief Get hallActive member.
 */
Direction_t Encoders::getHallActive() {
  return m_hallActive;
}

/*!
 * \brief Get carriage member.
 */
Carriage_t Encoders::getCarriage() {
  return m_carriage;
}

/*!
 * \brief Get machine type.
 */
Machine_t Encoders::getMachineType() {
  return m_machineType;
}

// Private Methods

Carriage_t Encoders::detectCarriageLeft() {
  uint16_t hallValue = analogRead(EOL_PIN_L);
  if (hallValue > FILTER_L_MAX[static_cast<uint8_t>(m_machineType)]) {
    return Carriage_t::Knit;
  } else if (hallValue < FILTER_L_MIN[static_cast<uint8_t>(m_machineType)]){
    return Carriage_t::Lace;
  }
  return Carriage_t::NoCarriage;
}

/*!
 * \brief Interrupt service subroutine.
 *
 * Called when encoder pin A is rising.
 * Must execute as fast as possible.
 * Bounds on `m_machineType` not checked.
 */
void Encoders::encA_rising() {
  // Update direction
  m_direction = digitalRead(ENC_PIN_B) != 0 ? Direction_t::Right : Direction_t::Left;

  // Update carriage position
  if (Direction_t::Right == m_direction) {
    m_position = m_position + (uint8_t) 1;

    // Reset carriage passed state when we know all magnets have cleared the turn mark.
    if (m_position > ALL_MAGNETS_CLEARED_LEFT[static_cast<uint8_t>(m_machineType)]) {
      m_passedLeft = false;
    }
  }

  // Scan for carriage in front of left Hall sensor
  Carriage_t detected_carriage = detectCarriageLeft();
  Carriage_t previous_detected_carriage = m_previousDetectedCarriageLeft;
  m_previousDetectedCarriageLeft = detected_carriage;

  // New carriage detected and headed to the right?
  if (Direction_t::Right == m_direction &&
      detected_carriage != Carriage_t::NoCarriage &&
      detected_carriage != previous_detected_carriage) {
    m_hallActive = Direction_t::Left;

    // Only set the belt shift the first time a magnet passes the turn mark.
    // Headed to the right.
    if (!m_passedLeft && Direction_t::Right == m_direction) {
      // Belt shift signal only decided in front of hall sensor
      m_beltShift = digitalRead(ENC_PIN_C) != 0 ? BeltShift::Shifted : BeltShift::Regular;
      m_passedLeft = true;

      if (Carriage_t::Garter == m_carriage) {
        // This has to be the first magnet and the belt shift needs to be swapped
        // But only for the G-carriage
        if (m_position < 30) {
          if (BeltShift::Regular == m_beltShift) {
            m_beltShift = BeltShift::Shifted;
          } else {
            m_beltShift = BeltShift::Regular;
          }
        }
      }
    }

    // The garter carriage has a second set of magnets that are going to
    // pass the sensor and will reset state incorrectly if allowed to
    // continue.
    if (Carriage_t::Garter == m_carriage) {
      return;
    }

    // If the carriage is already set, ignore the rest.
    if ((Carriage_t::Knit == m_carriage) && (Machine_t::Kh270 == m_machineType)) {
      return;
    }

    uint8_t start_position = END_LEFT_PLUS_OFFSET[static_cast<uint8_t>(m_machineType)];

    if (m_machineType == Machine_t::Kh270) {
      m_carriage = Carriage_t::Knit;

      // The first magnet on the carriage looks like Lace, the second looks like Knit
      if (detected_carriage == Carriage_t::Knit) {
        start_position = start_position + MAGNET_DISTANCE_270;
      }
    } else if (m_carriage == Carriage_t::NoCarriage) {
      m_carriage = detected_carriage;
    } else if (m_carriage != detected_carriage && m_position > start_position) {
      m_carriage = Carriage_t::Garter;

      // We swap the belt shift for the g-carriage because the point of work for 
      // the g-carraige is 13 needles behind the first magnet which puts it in a different
      // belt shift.
      // And we need to know the belt shift when the point of work is at needle 0.
      // Conveniently, the magnet distance on the K and L carraiges puts the point of
      // work within the same belt shift.
      if (BeltShift::Regular == m_beltShift) {
        m_beltShift = BeltShift::Shifted;
      } else {
        m_beltShift = BeltShift::Regular;
      }

      // Belt shift and start position were set when the first magnet passed
      // the sensor and we assumed we were working with a standard carriage.
      return;
    } else {
      m_carriage = detected_carriage;
    }

    // Known position of the carriage -> overwrite position
    m_position = start_position;
  }
}

/*!
 * \brief Interrupt service subroutine.
 *
 * Called when encoder pin A is falling.
 * Must execute as fast as possible.
 * Bounds on `m_machineType` not checked.
 */
void Encoders::encA_falling() {
  // Update direction
  m_direction = digitalRead(ENC_PIN_B) ? Direction_t::Left : Direction_t::Right;

  // Update carriage position
  if (Direction_t::Left == m_direction) {
    m_position = m_position - (uint8_t) 1;

    // Reset carriage passed state when we know all magnets have cleared the turn mark.
    if (m_position < ALL_MAGNETS_CLEARED_RIGHT[static_cast<uint8_t>(m_machineType)]) {
      m_passedRight = false;
    }
  }

  // In front of Right Hall Sensor?
  uint16_t hallValue = analogRead(EOL_PIN_R);

  // Avoid 'comparison of unsigned expression < 0 is always false'
  // by being explicit about that behaviour being expected.
  bool hallValueSmall = false;

  hallValueSmall = (hallValue < FILTER_R_MIN[static_cast<uint8_t>(m_machineType)]);

  if (hallValueSmall || hallValue > FILTER_R_MAX[static_cast<uint8_t>(m_machineType)]) {
    m_hallActive = Direction_t::Right;

    // Only set the belt shift when the first magnet passes the turn mark.
    // Headed to the left.
    if (!m_passedRight && Direction_t::Left == m_direction) {
      // Belt shift signal only decided in front of hall sensor
      m_beltShift = digitalRead(ENC_PIN_C) != 0 ? BeltShift::Regular : BeltShift::Shifted;
      m_passedRight = true;

      // Shift doens't need to be swapped for the g-carriage in this direction.
    }

    // The garter carriage has extra magnets that are going to
    // pass the sensor and will reset state incorrectly if allowed to
    // continue.
    if (m_carriage == Carriage_t::Garter) {
      return;
    }

    if (hallValueSmall) {
      m_carriage = Carriage_t::Knit;
    }

    // Known position of the carriage -> overwrite position
    m_position = END_RIGHT_MINUS_OFFSET[static_cast<uint8_t>(m_machineType)];
  }
}
