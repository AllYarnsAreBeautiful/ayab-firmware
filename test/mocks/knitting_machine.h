/*!
 * \file knitting_machine.h
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
 *    http://ayab-knitting.com
 */
#ifndef KNITTINGMACHINE_H_
#define KNITTINGMACHINE_H_

#include <cmath>
#include <cstdint>
#include <tuple>
#include <vector>

/**
 * The KnittingMachine class is a model of an electromechanical knitting
 * machine, without the firmware part. It does not know anything about the AYAB
 * code or the Arduino API.
 *
 * Its inputs are machine user actions (e.g. moving a carriage across the bed)
 * and electronic inputs (solenoid power status).
 *
 * Its outputs are the physical results of the inputs (e.g. positions of needles
 * after selection), and electronic outputs (encoder and position sensor
 * signals).
 */
class KnittingMachine {
public:
  /**
   * Return the state of the V1 encoder output
   */
  bool getEncoderOutput1() const;

  /**
   * Return the state of the V2 encoder output
   */
  bool getEncoderOutput2() const;

  /**
   * Return the state of the belt phase output
   */
  bool getBeltPhase() const;

  /**
   * Move belt 1/4 needle to the right
   */
  void moveBeltRight();

  /**
   * Move belt 1/4 needle to the left
   */
  void moveBeltLeft();

  /**
   * Get the voltage at the left position sensor
   */
  float getLeftPositionSensorVoltage();

  /**
   * Get the voltage at the right position sensor
   */
  float getRightPositionSensorVoltage();

  /**
   * Get the voltage at the right position sensor's K digital output
   * (low when a K carriage's magnet is detected, floating otherwise)
   */
  float getRightPositionSensorKSignal();

  /**
   * Add a magnet to the carriage.
   *
   * \param offsetFromCenter the position of the magnet (in needle widths)
   *                         given as distance from the carriage center
   *                         (positive = right)
   * \param polarity the magnet's polarity — (true = North, like K carriage)
   */
  void addCarriageMagnet(float offsetFromCenter, bool polarity);

  /**
   * Helper to add all G-carriage magnets (as measured on a KG-89)
   */
  void addGCarriageMagnets();

  /**
   * Set the carriage's position on the bed. This does not move the belt,
   * it is akin to directly inserting the carriage on the bed at the requested
   * position.
   *
   * \param position the new carriage position, in needles. 0 = carriage center
   *                 in front of the left position sensor (which
   *                 is also the position of needle 0 AKA L100).
   *                 Values outside of the machine's range of needles are
   *                 permitted.
   */
  void putCarriageCenterInFrontOfNeedle(int position);

  /**
   * Return the needle number the carriage center is the closest to.
   *
   * This may be outside of the machine's bed, e.g. -1 is one needle
   * width left of needle 0.
   */
  int getCarriageCenterNeedle();

  /**
   * Move the carriage to the right by 1/4 needle
   */
  void moveCarriageRight();

  /**
   * Move the carriage to the left by 1/4 needle
   */
  void moveCarriageLeft();

  /**
   * Move the carriage center to get closer to the target needle.
   *
   * \returns false if the carriage is already at the requested position
   */
  bool moveCarriageCenterTowardsNeedle(int position);

private:
  static constexpr int STEPS_PER_NEEDLE = 4;
  static constexpr float POSITION_SENSOR_LOW_VOLTAGE = 0.2f;
  static constexpr float POSITION_SENSOR_MID_VOLTAGE = 2.5f;
  static constexpr float POSITION_SENSOR_HIGH_VOLTAGE = 4.7f;

  /**
   * An internal type representing a position in 1/4 needle widths
   */
  struct qneedle_t {
    int value;
    bool operator==(const qneedle_t &other) const {
      return value == other.value;
    }
    bool operator!=(const qneedle_t &other) const {
      return !(*this == other);
    }
    bool operator>(const qneedle_t &other) const {
      return value > other.value;
    }
    qneedle_t operator-(const qneedle_t &other) const {
      return {value - other.value};
    }
    qneedle_t &operator++() {
      ++value;
      return *this;
    }
    qneedle_t &operator--() {
      --value;
      return *this;
    }
    float asNeedle() const {
      return static_cast<float>(value) / STEPS_PER_NEEDLE;
    }
    int closestNeedle() const {
      return std::round(asNeedle());
    }
    int leftNeedle() const {
      return std::floor(asNeedle());
    }
    static qneedle_t fromNeedle(float needle) {
      return {(int)std::round(needle * STEPS_PER_NEEDLE)};
    }
  };

  /**
   * How many steps the belt (and the rotary cams) goes through before ending up
   * in its original state.
   */
  int beltPeriod() const;

  /**
   * Tells whether the carriage's belt hooks are aligned with
   * the elongated belt holes, which are spaced <solenoid_count>/2
   * needles apart (on machines which have a belt shift), and
   * <solenoid_count> needles apart otherwise.
   */
  bool carriageEngagesBelt() const;

  /**
   * Get the voltage at a position sensor given its position
   */
  float getPositionSensorVoltage(qneedle_t sensorNeedlePosition) const;

  /**
   * Location of the position sensors (referenced to needle 0).
   */
  qneedle_t m_leftPositionSensorPosition = qneedle_t::fromNeedle(-0.5);
  qneedle_t m_rightPositionSensorPosition = qneedle_t::fromNeedle(200);

  /**
   * How far from the position sensor a magnet can be to be detected,
   * in needle widths.
   */
  float m_positionSensorRange = 0.75;

  /**
   * How many solenoids the machine has.
   */
  int m_solenoidCount = 16;

  /**
   * Does the machine have a concept of belt shift, i.e. intermediate
   * elongated holes on the belt?
   */
  bool m_hasBeltShift = true;

  /**
   * It the machine has two belt phases, at which point in the belt
   * cycle does the phase change?
   * Given in encoder steps, i.e. 1/4 needle widths.
   */
  int m_beltPhaseOffset = 9;

  /**
   * We only store the belt position modulo <solenoid count> * 4.
   *
   * Its unit is an encoder step, i.e. 1/4 of a needle width.
   *
   * A belt position of 0 represents a belt position where an
   * elongated hole is in front of the left position sensor.
   */
  std::uint8_t m_beltPosition = 0;

  /**
   * The carriage position in 1/4 needles.
   *
   * Position 0 is when the carriage center is over needle 0.
   */
  qneedle_t m_carriagePosition = qneedle_t::fromNeedle(-32);

  /**
   * A list of carriage magnets, defined by their offset from the carriage
   * center and their polarity. \see addCarriageMagnet()
   */
  std::vector<std::pair<float, bool>> m_carriageMagnets;
};

#endif // KNITTINGMACHINE_H_