/*!
 * \file encoders.h
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
 *    Copyright 2013 Christian Obersteiner, Andreas Müller
 *    http://ayab-knitting.com
 */

#ifndef ENCODERS_H_
#define ENCODERS_H_

#include <Arduino.h>

// Should be calibrated to each device
// These values are for the K carriage
#if defined(KH910)
#define FILTER_L_MIN 200 // below: L Carriage
#define FILTER_L_MAX 600 // above: K Carriage
#define FILTER_R_MIN 200
#define FILTER_R_MAX 1023
#elif defined(KH930)
#define FILTER_L_MIN 200 // below: L Carriage
#define FILTER_L_MAX 600 // above: K Carriage
#define FILTER_R_MIN 0
#define FILTER_R_MAX 600
#else
#error "KH910 or KH930 has to be defined as a preprocessor variable!"
#endif

#define END_LEFT 0
#define END_RIGHT 255

typedef enum Direction { NoDirection = 0, Left = 1, Right = 2 } Direction_t;

typedef enum Carriage { NoCarriage = 0, K = 1, L = 2, G = 3 } Carriage_t;

typedef enum Beltshift {
  Unknown = 0,
  Regular = 1,
  Shifted = 2,
  Lace_Regular = 3,
  Lace_Shifted = 4
} Beltshift_t;

/*!
 * \brief Encoder interface.
 *
 * Encoders for Beltshift, Direction, Active Hall sensor and Carriage Type.
 */
class Encoders {
public:
  Encoders();

  void encA_interrupt();

  uint8_t getPosition();
  Beltshift_t getBeltshift();
  Direction_t getDirection();
  Direction_t getHallActive();
  Carriage_t getCarriage();

  uint16_t getHallValue(Direction_t);

private:
  Direction_t m_direction = NoDirection;
  Direction_t m_hallActive = NoDirection;
  Beltshift_t m_beltShift = Unknown;
  Carriage_t m_carriage = NoCarriage;
  uint8_t m_encoderPos = 0x00;
  bool _oldState = false;

  void encA_rising();
  void encA_falling();
};

#endif // ENCODERS_H_
