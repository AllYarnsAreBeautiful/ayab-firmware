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
#if defined(KH910) || defined(AYAB_HW_TEST)
constexpr uint16_t FILTER_L_MIN = 200U; // below: L Carriage
constexpr uint16_t FILTER_L_MAX = 600U; // above: K Carriage
constexpr uint16_t FILTER_R_MIN = 200U;
constexpr uint16_t FILTER_R_MAX = 1023U;
#elif defined(KH930)
constexpr uint16_t FILTER_L_MIN = 200U; // below: L Carriage
constexpr uint16_t FILTER_L_MAX = 600U; // above: K Carriage
constexpr uint16_t FILTER_R_MIN = 0U;
constexpr uint16_t FILTER_R_MAX = 600U;
#else
#error "KH910 or KH930 has to be defined as a preprocessor variable!"
#endif

constexpr uint8_t END_LEFT = 0U;
constexpr uint8_t END_RIGHT = 255U;
constexpr uint8_t END_OFFSET = 28;

enum Direction { NoDirection, Left, Right, NUM_DIRECTIONS };

enum Carriage { NoCarriage, K, L, G, NUM_CARRIAGES };
enum Beltshift {
  Unknown,
  Regular,
  Shifted,
  Lace_Regular,
  Lace_Shifted,
  NUM_BELTSHIFTS
};

using Direction_t = enum Direction;

using Carriage_t = enum Carriage;

using Beltshift_t = enum Beltshift;

/*!
 * \brief Encoder interface.
 *
 * Encoders for Beltshift, Direction, Active Hall sensor and Carriage Type.
 */
class Encoders {
public:
  Encoders() = default;

  void encA_interrupt();

  auto getPosition() const -> uint8_t;
  auto getBeltshift() -> Beltshift_t;
  auto getDirection() -> Direction_t;
  auto getHallActive() -> Direction_t;
  auto getCarriage() -> Carriage_t;

  static auto getHallValue(Direction_t pSensor) -> uint16_t;

private:
  Direction_t m_direction = NoDirection;
  Direction_t m_hallActive = NoDirection;
  Beltshift_t m_beltShift = Unknown;
  Carriage_t m_carriage = NoCarriage;
  uint8_t m_encoderPos = 0x00;
  bool m_oldState = false;

  void encA_rising();
  void encA_falling();
};

#endif // ENCODERS_H_
