/*!
 * \file knitting_machine.cpp
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
#include <cmath>
#include <tuple>
#include <vector>

#include "knitting_machine.h"

bool KnittingMachine::getEncoderOutput1() const {
  return (m_beltPosition + 1) % STEPS_PER_NEEDLE >= 2;
}

bool KnittingMachine::getEncoderOutput2() const {
  return m_beltPosition % STEPS_PER_NEEDLE <= 1;
}

bool KnittingMachine::getBeltPhase() const {
  if (!m_hasBeltShift) {
    return false;
  }
  return ((m_beltPosition + beltPeriod() + m_beltPhaseOffset) % beltPeriod()) >=
         (beltPeriod() / 2);
}

void KnittingMachine::moveBeltRight() {
  m_beltPosition = (m_beltPosition + 1) % beltPeriod();
}

void KnittingMachine::moveBeltLeft() {
  m_beltPosition = (m_beltPosition + (beltPeriod() - 1)) % beltPeriod();
}

float KnittingMachine::getPositionSensorVoltage(
    qneedle_t sensorNeedlePos) const {
  float sensorPosition = sensorNeedlePos.asNeedle();
  for (std::pair<float, bool> magnet : m_carriageMagnets) {
    float magnetPosition = m_carriagePosition.asNeedle() + magnet.first;
    if (std::abs(sensorPosition - magnetPosition) <= m_positionSensorRange) {
      return magnet.second ? POSITION_SENSOR_HIGH_VOLTAGE
                           : POSITION_SENSOR_LOW_VOLTAGE;
    }
  }
  return POSITION_SENSOR_MID_VOLTAGE;
}

float KnittingMachine::getLeftPositionSensorVoltage() {
  return getPositionSensorVoltage(m_leftPositionSensorPosition);
}

float KnittingMachine::getRightPositionSensorVoltage() {
  return getPositionSensorVoltage(m_rightPositionSensorPosition);
}

float KnittingMachine::getRightPositionSensorKSignal() {
  return getRightPositionSensorVoltage() >= POSITION_SENSOR_HIGH_VOLTAGE
             ? POSITION_SENSOR_LOW_VOLTAGE
             : POSITION_SENSOR_MID_VOLTAGE;
}

void KnittingMachine::addCarriageMagnet(float offsetFromCenter, bool polarity) {
  m_carriageMagnets.push_back(std::make_pair(offsetFromCenter, polarity));
}

void KnittingMachine::addGCarriageMagnets() {
  addCarriageMagnet(12.25, false);
  addCarriageMagnet(10.5, true);
  addCarriageMagnet(-10.5, true);
  addCarriageMagnet(-12.25, false);
}

void KnittingMachine::putCarriageCenterInFrontOfNeedle(int position) {
  m_carriagePosition =
      qneedle_t::fromNeedle(position); // convert to 1/4 of needles
}

int KnittingMachine::getCarriageCenterNeedle() {
  return m_carriagePosition.closestNeedle();
}

bool KnittingMachine::carriageEngagesBelt() const {
  // TODO disengage carriage when it moves outside the bed
  // TODO simulate carriage belt hooks (currently simulates
  //      a single hook at carriage center)
  // TODO simulate belt slack
  int period = STEPS_PER_NEEDLE * m_solenoidCount;
  if (m_hasBeltShift) {
    period /= 2;
  }
  return (m_beltPosition % period) ==
         (m_carriagePosition.value % period + period) % period;
}

void KnittingMachine::moveCarriageRight() {
  if (carriageEngagesBelt()) {
    moveBeltRight();
  }
  ++m_carriagePosition;
}

void KnittingMachine::moveCarriageLeft() {
  if (carriageEngagesBelt()) {
    moveBeltLeft();
  }
  --m_carriagePosition;
}

int KnittingMachine::beltPeriod() const {
  return m_solenoidCount * STEPS_PER_NEEDLE;
}

bool KnittingMachine::moveCarriageCenterTowardsNeedle(int position) {
  qneedle_t target = qneedle_t::fromNeedle(position);
  if (m_carriagePosition == target) {
    return false;
  }
  if (target > m_carriagePosition) {
    moveCarriageRight();
  } else {
    moveCarriageLeft();
  }
  return true;
}