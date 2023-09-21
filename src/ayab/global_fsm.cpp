/*!
 * \file global_fsm.cpp
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
 *    Modified Work Copyright 2020-3 Sturla Lange, Tom Price
 *    http://ayab-knitting.com
 */

#include "fsm.h"

// static member functions

void GlobalFsm::init() {
  m_instance->init();
}

void GlobalFsm::update() {
  m_instance->update();
}

void GlobalFsm::cacheEncoders() {
  m_instance->cacheEncoders();
}

void GlobalFsm::setState(OpInterface* state) {
  m_instance->setState(state);
}

OpInterface *GlobalFsm::getState() {
  return m_instance->getState();
}

void GlobalFsm::setMachineType(Machine_t machineType) {
  m_instance->setMachineType(machineType);
}

Machine_t GlobalFsm::getMachineType() {
  return m_instance->getMachineType();
}

BeltShift_t GlobalFsm::getBeltShift() {
  return m_instance->getBeltShift();
}

Carriage_t GlobalFsm::getCarriage() {
  return m_instance->getCarriage();
}

Direction_t GlobalFsm::getDirection() {
  return m_instance->getDirection();
}

Direction_t GlobalFsm::getHallActive() {
  return m_instance->getHallActive();
}

uint8_t GlobalFsm::getPosition() {
  return m_instance->getPosition();
}
