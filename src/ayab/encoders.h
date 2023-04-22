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

enum BeltShift { Unknown, Regular, Shifted, Lace_Regular, Lace_Shifted };
using BeltShift_t = enum BeltShift;

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

constexpr uint8_t GARTER_SLOP = 14;

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
 * Encoders for BeltShift, Direction, Active Hall sensor and Carriage Type.
 */
class EncodersInterface {
public:
  virtual ~EncodersInterface(){};

  // any methods that need to be mocked should go here
  virtual void encA_interrupt() = 0;
  virtual uint16_t getHallValue(Direction_t pSensor) = 0;

  virtual void init(Machine_t machineType) = 0;
  virtual Machine_t getMachineType() = 0;

  // virtual void setBeltShift(BeltShift_t beltShift) = 0;
  virtual BeltShift_t getBeltShift() = 0;

  // virtual void setCarriage(Carriage_t carriage) = 0;
  virtual Carriage_t getCarriage() = 0;

  // virtual void setDirection(Direction_t direction) = 0;
  virtual Direction_t getDirection() = 0;

  // virtual void setHallActive(Direction_t hallActive) = 0;
  virtual Direction_t getHallActive() = 0;

  // virtual void setPosition(uint8_t position) = 0;
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

  // static void setBeltShift(BeltShift_t beltShift);
  static BeltShift_t getBeltShift();

  // static void setCarriage(Carriage_t carriage);
  static Carriage_t getCarriage();

  // static void setDirection(Direction_t direction);
  static Direction_t getDirection();

  // static void setHallActive(Direction_t hallActive);
  static Direction_t getHallActive();

  // static void setPosition(uint8_t position);
  static uint8_t getPosition();
};

class Encoders : public EncodersInterface {
public:
  Encoders() = default;

  void encA_interrupt();
  uint16_t getHallValue(Direction_t pSensor);

  void init(Machine_t machineType);
  Machine_t getMachineType();

  // void setBeltShift(BeltShift_t beltShift);
  BeltShift_t getBeltShift();

  // void setCarriage(Carriage_t carriage);
  Carriage_t getCarriage();

  // void setDirection(Direction_t direction);
  Direction_t getDirection();

  // void setHallActive(Direction_t hallActive);
  Direction_t getHallActive();

  // void setPosition(uint8_t position);
  uint8_t getPosition();

private:
  Machine_t m_machineType;

  volatile BeltShift_t m_beltShift;
  volatile Carriage_t m_carriage;
  volatile Direction_t m_direction;
  volatile Direction_t m_hallActive;
  volatile uint8_t m_position;
  volatile bool m_oldState;

  void encA_rising();
  void encA_falling();
};

#endif // ENCODERS_H_
