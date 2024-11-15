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
#include <algorithm>
#include <cassert>
#include <cmath>
#include <string>
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
  return (m_beltPosition + m_beltPhaseOffset) >= (m_beltPosition.period / 2);
}

void KnittingMachine::moveBeltRight() {
  ++m_beltPosition;
  for (auto &solenoid : m_solenoids) {
    ++solenoid.m_phase;
  }
}

void KnittingMachine::moveBeltLeft() {
  --m_beltPosition;
  for (auto &solenoid : m_solenoids) {
    --solenoid.m_phase;
  }
}

float KnittingMachine::getPositionSensorVoltage(
    qneedle_t sensorNeedlePos) const {
  float sensorPosition = sensorNeedlePos.asNeedle();
  for (std::pair<float, bool> magnet : m_carriage.m_magnets) {
    float magnetPosition = m_carriage.m_position.asNeedle() + magnet.first;
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
  m_carriage.m_magnets.push_back(std::make_pair(offsetFromCenter, polarity));
}

void KnittingMachine::addGCarriageMagnets() {
  addCarriageMagnet(12.25, false);
  addCarriageMagnet(10.5, true);
  addCarriageMagnet(-10.5, true);
  addCarriageMagnet(-12.25, false);
}

void KnittingMachine::setCarriageHookDistance(int distance) {
  assert(distance >= 0);
  m_carriage.m_hookDistance = distance;
}

void KnittingMachine::setCarriageNeedleTestDistance(int distance) {
  assert(distance >= 0);
  m_carriage.m_needleTestDistance = distance;
}

void KnittingMachine::putCarriageCenterInFrontOfNeedle(int position) {
  m_carriage.m_position =
      qneedle_t::fromNeedle(position); // convert to 1/4 of needles
}

int KnittingMachine::getCarriageCenterNeedle() {
  return m_carriage.m_position.closestNeedle();
}

bool KnittingMachine::carriageEngagesBelt() const {
  // TODO disengage carriage when it moves outside the bed
  // TODO simulate belt slack
  int period = m_beltPosition.period;
  if (m_hasBeltShift) {
    period /= 2;
  }
  return modular_t(m_beltPosition, period) ==
         modular_t(m_carriage.m_position.value +
                       m_carriage.m_hookDistance * STEPS_PER_NEEDLE,
                   period);
}

void KnittingMachine::moveCarriageRight() {
  if (carriageEngagesBelt()) {
    moveBeltRight();
  }
  m_carriage.moveRight();

  updateSolenoids();
  updateNeedles();
}

void KnittingMachine::moveCarriageLeft() {
  if (carriageEngagesBelt()) {
    moveBeltLeft();
  }
  m_carriage.moveLeft();

  updateSolenoids();
  updateNeedles();
}

int KnittingMachine::beltPeriod() const {
  return m_solenoidCount * STEPS_PER_NEEDLE;
}

bool KnittingMachine::moveCarriageCenterTowardsNeedle(int position) {
  qneedle_t target = qneedle_t::fromNeedle(position);
  if (m_carriage.m_position == target) {
    return false;
  }
  if (target > m_carriage.m_position) {
    moveCarriageRight();
  } else {
    moveCarriageLeft();
  }
  return true;
}

void KnittingMachine::setSolenoidCount(int count) {
  m_solenoids.clear();
  m_solenoidCount = count;
  modular_t phase(0, count * STEPS_PER_NEEDLE);
  for (int n = 0; n < count; n++) {
    m_solenoids.push_back(Solenoid{.m_index = n,
                                   .m_isEnergized = false,
                                   .m_phase = phase,
                                   .m_isHoldingRodDown = false});

    // If solenoid 0 drives needle 0, solenoid 1 drives needle 1.
    // Solenoid (cam) phases will be increased by STEPS_PER_NEEDLE as the
    // carriage moves one needle to the right. When the carriage gets to needle
    // 1, the cam phase for solenoid 1 should be equal to the cam phase for
    // solenoid 0 back when the carriage was at needle 0.
    // So the phase for solenoid 1 should always be STEPS_PER_NEEDLE "behind"
    // the phase for solenoid 0.
    phase -= STEPS_PER_NEEDLE;
  }
  // each plate is connected to two solenoids count/2 apart
  for (int n = 0; n < count / 2; n++) {
    m_selectorPlates.push_back(
        SelectorPlate{.m_index = n,
                      .m_solenoid1 = m_solenoids[n],
                      .m_solenoid2 = m_solenoids[n + count / 2]});
  }
}

void KnittingMachine::setNeedleCount(int count) {
  m_needles.clear();
  for (int n = 0; n < count; n++) {
    m_needles.push_back(Needle{
        .m_index = n,
        .m_position = NeedlePosition::A,
        .m_selectorPlate = m_selectorPlates[n % (m_selectorPlates.size())],
        .m_carriage = m_carriage});
  }
}

void KnittingMachine::setNeedlePosition(int needle, NeedlePosition position) {
  m_needles[needle].m_position = position;
}

void KnittingMachine::setNeedlePositions(int startNeedle,
                                         std::string positions) {
  if (startNeedle < 0 || positions.empty() ||
      size_t(startNeedle + positions.length()) > m_needles.size()) {
    return;
  }

  for (size_t i = 0; i < positions.length(); ++i) {
    char c = positions[i];
    if (c < 'A' || c > 'E') {
      continue;
    }
    m_needles[startNeedle + i].m_position = NeedlePosition(c - 'A');
  }
}

KnittingMachine::NeedlePosition KnittingMachine::getNeedlePosition(int needle) {
  return m_needles[needle].m_position;
}

std::string KnittingMachine::getNeedlePositions(int startNeedle,
                                                int needleCount) {
  if (needleCount < 0) {
    needleCount = m_needles.size() - startNeedle - 1;
  }
  if (startNeedle < 0 || needleCount <= 0 ||
      static_cast<size_t>(startNeedle + needleCount) > m_needles.size()) {
    return "";
  }

  std::string positions(needleCount, '\0');

  std::transform(m_needles.begin() + startNeedle,
                 m_needles.begin() + startNeedle + needleCount,
                 positions.begin(), [](const Needle &needle) {
                   return static_cast<char>('A' + needle.m_position);
                 });

  return positions;
}

void KnittingMachine::setSolenoid(int solenoid, bool isEnergized) {
  if (solenoid < 0 || size_t(solenoid) >= m_solenoids.size()) {
    return;
  }
  m_solenoids[solenoid].m_isEnergized = isEnergized;
}

void KnittingMachine::updateSolenoids() {
  for (auto &solenoid : m_solenoids) {
    solenoid.update();
  }
}

void KnittingMachine::updateNeedles() {
  for (auto &needle : m_needles) {
    needle.update();
  }
}

void KnittingMachine::Carriage::moveRight() {
  ++m_position;
}

void KnittingMachine::Carriage::moveLeft() {
  --m_position;
}

bool KnittingMachine::Solenoid::isPushingRodDown() const {
  // The rotary cam is only pushing the rod down at the start and end of its
  // cycle
  return m_phase < (1 * m_phase.period / 6) ||
         m_phase > (5 * m_phase.period / 6);
}

void KnittingMachine::Solenoid::update() {
  // The rod can only start or stop following the cam while it is
  // being pushed down by the cam — otherwise, the solenoid is not
  // strong enough to pull it down if it is being powered on, and
  // if it is being powered off, the rod will be released but won't
  // start riding the side-pushing part of the cam until the next
  // cycle.
  if (isPushingRodDown()) {
    m_isHoldingRodDown = m_isEnergized;
  }
}

bool KnittingMachine::Solenoid::isPushingPlate() const {
  // If the rod is being held away from the cam, the selecting plate
  // won't be pushed.
  if (m_isHoldingRodDown) {
    return false;
  }
  // The rotary cam is only pushing the plate in the middle of its cycle
  return m_phase > (2 * m_phase.period / 6) &&
         m_phase < (4 * m_phase.period / 6);
}

bool KnittingMachine::SelectorPlate::isHookingNeedles() const {
  return m_solenoid1.isPushingPlate() || m_solenoid2.isPushingPlate();
}

void KnittingMachine::Needle::update() {
  const int carriageOffset =
      std::abs(m_carriage.m_position.closestNeedle() - m_index);

  if (carriageOffset == 0) {
    // Simulate knitting cams by moving all needles to C
    // at carriage center, before they encounter the selection cams.
    // An actual carriage moves the needle through the knitting
    // positions, but always leaves it (when in patterning mode)
    // in the C position where the needle pressing cam lives.
    m_position = C;
  }

  // Check if the needle is in the right position to be pressed by the
  // presser cam (this serves to avoid needles going through
  // the selector area "the wrong way", i.e. on the leading edge
  // of carriage travel)
  if (m_position == C) {
    // As the presser releases the needle…
    if (carriageOffset == m_carriage.m_needleTestDistance) {
      // If the plate is hooking the needle…
      if (m_selectorPlate.isHookingNeedles()) {
        // It ends up in D
        m_position = D;
      } else {
        // Otherwise, it goes back up into B
        m_position = B;
      }
    }
  }
}