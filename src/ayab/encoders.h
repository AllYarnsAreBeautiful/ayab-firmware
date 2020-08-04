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
 *    Original Work Copyright 2013 Christian Obersteiner, Andreas Müller
 *    Modified Work Copyright 2020 Sturla Lange, Tom Price
 *    http://ayab-knitting.com
 */

// TODO(TP): incorporate into machine instead of knitter

#ifndef ENCODERS_H_
#define ENCODERS_H_

#include <Arduino.h>

// Enumerated constants

enum Direction { NoDirection = -1, Left = 0, Right = 1, NUM_DIRECTIONS = 2 };
using Direction_t = enum Direction;

enum Carriage {
  NoCarriage = -1,
  Knit = 0,
  Lace = 1,
  Garter = 2,
  NUM_CARRIAGES = 3
};
using Carriage_t = enum Carriage;

enum Beltshift { Unknown, Regular, Shifted, Lace_Regular, Lace_Shifted };
using Beltshift_t = enum Beltshift;

enum MachineType {
  NoMachine = -1,
  Kh910 = 0,
  Kh930 = 1,
  Kh270 = 2,
  NUM_MACHINES = 3
};
using Machine_t = enum MachineType;

// Machine constants

constexpr uint8_t NUM_NEEDLES[NUM_MACHINES] = {200, 200, 114};
constexpr uint8_t LINE_BUFFER_LEN[NUM_MACHINES] = {25, 25, 15};
constexpr uint8_t END_OF_LINE_OFFSET_L[NUM_MACHINES] = {12, 12, 6};
constexpr uint8_t END_OF_LINE_OFFSET_R[NUM_MACHINES] = {12, 12, 6};

constexpr uint8_t END_LEFT[NUM_MACHINES] = {0U, 0U, 0U};
constexpr uint8_t END_RIGHT[NUM_MACHINES] = {255U, 255U, 140U};
constexpr uint8_t END_OFFSET[NUM_MACHINES] = {28U, 28U, 14U};

constexpr uint8_t START_OFFSET[NUM_MACHINES][NUM_DIRECTIONS][NUM_CARRIAGES] = {
    // KH910
    {
        //   K,  L,  G
        {40, 40, 8}, // Left
        {16, 16, 32} // Right
    },
    // KH930
    {
        //   K,  L,  G
        {40, 40, 8}, // Left
        {16, 16, 32} // Right
    },
    // KH270
    {
        //   K
        {14, 0, 0}, // Left
        {2, 0, 0}   // Right
    }};

// Should be calibrated to each device
// Below filter minimum -> Lace carriage
// Above filter maximum -> Knit carriage
//                                               KH910 KH930 KH270
constexpr uint16_t FILTER_L_MIN[NUM_MACHINES] = {200U, 200U, 200U};
constexpr uint16_t FILTER_L_MAX[NUM_MACHINES] = {600U, 600U, 600U};
constexpr uint16_t FILTER_R_MIN[NUM_MACHINES] = {200U, 0U, 0U};
constexpr uint16_t FILTER_R_MAX[NUM_MACHINES] = {1023U, 600U, 600U};

constexpr uint16_t SOLENOIDS_BITMASK = 0xFFFFU;

/*!
 * \brief Encoder interface.
 *
 * Encoders for Beltshift, Direction, Active Hall sensor and Carriage Type.
 */
class Encoders {
public:
  Encoders() = default;

  void encA_interrupt();

  static uint16_t getHallValue(Direction_t pSensor);

  // getter/setter functions to assist mocking
  uint8_t getPosition() const;
  Beltshift_t getBeltshift() const;
  Direction_t getDirection() const;
  Direction_t getHallActive() const;
  Carriage_t getCarriage() const;
  Machine_t getMachineType() const;
  void init(Machine_t machineType);

private:
  Direction_t m_direction = NoDirection;
  Direction_t m_hallActive = NoDirection;
  Beltshift_t m_beltShift = Unknown;
  Carriage_t m_carriage = NoCarriage;
  Machine_t m_machineType = NoMachine;

  uint8_t m_encoderPos = 0x00;
  bool m_oldState = false;

  void encA_rising();
  void encA_falling();
};

#endif // ENCODERS_H_
