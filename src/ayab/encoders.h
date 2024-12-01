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

#ifndef ENCODERS_H_
#define ENCODERS_H_

#include <Arduino.h>

// Enumerated constants


enum class Direction : unsigned char {
  NoDirection = 0xFF,
  Left = 0,
  Right = 1
};
constexpr int NUM_DIRECTIONS = 2;
using Direction_t = enum Direction;

enum class Carriage : unsigned char {
  NoCarriage = 0xFF,
  Knit = 0,
  Lace = 1,
  Garter = 2
};
constexpr int NUM_CARRIAGES = 3;
using Carriage_t = enum Carriage;

enum class MachineType : unsigned char {
  NoMachine = 0xFF,
  Kh910 = 0,
  Kh930 = 1,
  Kh270 = 2
};
constexpr int NUM_MACHINES = 3;
using Machine_t = enum MachineType;

enum class BeltShift : unsigned char { Unknown, Regular, Shifted, Lace_Regular, Lace_Shifted };
using BeltShift_t = enum BeltShift;

// Machine constants

constexpr uint8_t NUM_NEEDLES[NUM_MACHINES] = {200U, 200U, 112U};
constexpr uint8_t LINE_BUFFER_LEN[NUM_MACHINES] = {25U, 25U, 14U};
constexpr uint8_t END_OF_LINE_OFFSET_L[NUM_MACHINES] = {12U, 12U, 6U};
constexpr uint8_t END_OF_LINE_OFFSET_R[NUM_MACHINES] = {12U, 12U, 6U};

constexpr uint8_t END_LEFT[NUM_MACHINES] = {0U, 0U, 0U};
constexpr uint8_t END_RIGHT[NUM_MACHINES] = {255U, 255U, 140U};
constexpr uint8_t END_OFFSET[NUM_MACHINES] = {28U, 28U, 5U};

// The following two arrays are created by combining, respectively,
// the arrays END_LEFT and END_RIGHT with END_OFFSET
constexpr uint8_t END_LEFT_PLUS_OFFSET[NUM_MACHINES] = {28U, 28U, 5U};
constexpr uint8_t END_RIGHT_MINUS_OFFSET[NUM_MACHINES] = {227U, 227U, 135U};

constexpr uint8_t ALL_MAGNETS_CLEARED_LEFT[NUM_MACHINES] = {56U, 56U, 10U};
constexpr uint8_t ALL_MAGNETS_CLEARED_RIGHT[NUM_MACHINES] = {199U, 199U, 130U};

// The garter slop is needed to determine whether or not we have a garter carriage.
// If we didn't have it, we'd decide which carriage we had when the first magnet passed the sensor.
// For the garter carriage we need to see both magnets.
constexpr uint8_t GARTER_SLOP = 2U;

// How much earlier than theoretically ideal should we switch solenoids (in needles)
constexpr int TIMING_ADVANCE_9XX = 4;
constexpr int TIMING_ADVANCE_270 = 4;

constexpr uint8_t MAGNET_DISTANCE_270 = 12U;

constexpr uint8_t START_OFFSET[NUM_MACHINES][NUM_DIRECTIONS][NUM_CARRIAGES] = {
    // KH910: 16 solenoids, half-cycle is 8 needles
    //  - K: selectors 24 needles away from center -> solenoid activation points 16 needles away from center
    //  - L: selectors 12 needles away from center -> solenoid activation points 4 needles away from center
    //  - G: selector at center -> solenoid activation points 8 needles away from center
    //  — add END_OFFSET
    //  - for G add magnet distance from center (12)
    {
        // K,                           L,                            G
        {END_OFFSET[0] + (16 - TIMING_ADVANCE_9XX), END_OFFSET[0] + (4 - TIMING_ADVANCE_9XX), END_OFFSET[0] - (8 - TIMING_ADVANCE_9XX) + 12}, // Left
        {END_OFFSET[0] - (16 - TIMING_ADVANCE_9XX), END_OFFSET[0] - (4 - TIMING_ADVANCE_9XX), END_OFFSET[0] + (8 - TIMING_ADVANCE_9XX) + 12} // Right
    },
    // KH930: same as KH910
    {
        // K,                           L,                            G
        {END_OFFSET[1] + (16 - TIMING_ADVANCE_9XX), END_OFFSET[1] + (4 - TIMING_ADVANCE_9XX), END_OFFSET[1] - (8 - TIMING_ADVANCE_9XX) + 12}, // Left
        {END_OFFSET[1] - (16 - TIMING_ADVANCE_9XX), END_OFFSET[1] - (4 - TIMING_ADVANCE_9XX), END_OFFSET[1] + (8 - TIMING_ADVANCE_9XX) + 12} // Right
    },
    // KH270: 12 solenoids, half-cycle is 6 needles
    //  - K: selectors 15 needles away from center -> solenoid activation points 9 needles away
    //  - add END_OFFSET
    //  - add MAGNET_DISTANCE_270
    //  - add actual magnet distance from center (3)
    {
        // K
        {END_OFFSET[2] + (9 - TIMING_ADVANCE_270) + MAGNET_DISTANCE_270 + 3}, // Left
        {END_OFFSET[2] - (9 - TIMING_ADVANCE_270) + MAGNET_DISTANCE_270 + 3} // Right
    }};

// Should be calibrated to each device
// Below filter minimum -> Lace carriage
// Above filter maximum -> Knit carriage
//                                               KH910 KH930 KH270
constexpr uint16_t FILTER_L_MIN[NUM_MACHINES] = { 200U, 200U, 200U};
constexpr uint16_t FILTER_L_MAX[NUM_MACHINES] = { 600U, 600U, 600U};
constexpr uint16_t FILTER_R_MIN[NUM_MACHINES] = { 200U,   0U,   0U};
constexpr uint16_t FILTER_R_MAX[NUM_MACHINES] = {1023U, 600U, 600U};

constexpr uint16_t SOLENOIDS_BITMASK = 0xFFFFU;

/*!
 * \brief Encoder interface.
 *
 * Encoders for BeltShift, Direction, Active Hall sensor and Carriage Type.
 */
class EncodersInterface {
public:
  virtual ~EncodersInterface() = default;

  // any methods that need to be mocked should go here
  virtual void encA_interrupt() = 0;
  virtual uint16_t getHallValue(Direction_t pSensor) = 0;
  virtual void init(Machine_t machineType) = 0;
  virtual Machine_t getMachineType() = 0;
  virtual BeltShift_t getBeltShift() = 0;
  virtual Carriage_t getCarriage() = 0;
  virtual Direction_t getDirection() = 0;
  virtual Direction_t getHallActive() = 0;
  virtual uint8_t getPosition() = 0;
};

// Container class for the static methods for the encoders.
// Dependency injection is enabled using a pointer to a global instance of
// either `Encoders` or `EncodersMock`, both of which classes implement the
// pure virtual methods of `EncodersInterface`.

class GlobalEncoders final {
private:
  // singleton class so private constructor is appropriate
  GlobalEncoders() = default;

public:
  // pointer to global instance whose methods are implemented
  static EncodersInterface *m_instance;

  static void encA_interrupt();
  static uint16_t getHallValue(Direction_t pSensor);
  static void init(Machine_t machineType);
  static Machine_t getMachineType();
  static BeltShift_t getBeltShift();
  static Carriage_t getCarriage();
  static Direction_t getDirection();
  static Direction_t getHallActive();
  static uint8_t getPosition();
};

class Encoders : public EncodersInterface {
public:
  Encoders() = default;

  void encA_interrupt() final;
  uint16_t getHallValue(Direction_t pSensor) final;
  void init(Machine_t machineType) final;
  Machine_t getMachineType() final;
  BeltShift_t getBeltShift() final;
  Carriage_t getCarriage() final;
  Direction_t getDirection() final;
  Direction_t getHallActive() final;
  uint8_t getPosition() final;

private:
  Machine_t m_machineType;

  volatile BeltShift_t m_beltShift;
  volatile Carriage_t m_carriage;
  volatile Direction_t m_direction;
  volatile Direction_t m_hallActive;
  volatile uint8_t m_position;
  volatile bool m_oldState;
  volatile bool m_passedLeft;
  volatile bool m_passedRight;

  void encA_rising();
  void encA_falling();
};

#endif // ENCODERS_H_
