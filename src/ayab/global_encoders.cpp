/*!
 * \file global_encoders.cpp
 * \brief Singleton class containing methods governing encoders
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
 *    Original Work Copyright 2013 Christian Obersteiner, Andreas Müller
 *    Modified Work Copyright 2020 Sturla Lange, Tom Price
 *    http://ayab-knitting.com
 */

#include "encoders.h"

void GlobalEncoders::encA_interrupt() {
  m_instance->encA_interrupt();
}

uint16_t GlobalEncoders::getHallValue(Direction_t pSensor) {
  return m_instance->getHallValue(pSensor);
}

void GlobalEncoders::init(Machine_t machineType) {
  m_instance->init(machineType);
}

/*
Machine_t GlobalEncoders::getMachineType() {
  return m_instance->getMachineType();
}

void GlobalEncoders::setBeltShift(BeltShift_t beltShift) {
  m_instance->setBeltShift(beltShift);
}
*/

BeltShift_t GlobalEncoders::getBeltShift() {
  return m_instance->getBeltShift();
}

/*
void GlobalEncoders::setCarriage(Carriage_t carriage) {
  m_instance->setCarriage(carriage);
}
*/

Carriage_t GlobalEncoders::getCarriage() {
  return m_instance->getCarriage();
}

/*
void GlobalEncoders::setDirection(Direction_t direction) {
  m_instance->setDirection(direction);
}
*/

Direction_t GlobalEncoders::getDirection() {
  return m_instance->getDirection();
}

/*
void GlobalEncoders::setHallActive(Direction_t hallActive) {
  m_instance->setHallActive(hallActive);
}
*/

Direction_t GlobalEncoders::getHallActive() {
  return m_instance->getHallActive();
}

/*
void GlobalEncoders::setPosition(uint8_t position) {
  m_instance->setPosition(position);
}
*/

uint8_t GlobalEncoders::getPosition() {
  return m_instance->getPosition();
}
