/*!
 * \file global_op.cpp
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

#include "op.h"

// static member functions

void GlobalOp::init() {
  m_instance->init();
}

void GlobalOp::update() {
  m_instance->update();
}

void GlobalOp::cacheEncoders() {
  m_instance->cacheEncoders();
}

void GlobalOp::setState(OpState_t state) {
  m_instance->setState(state);
}

OpState_t GlobalOp::getState() {
  return m_instance->getState();
}

BeltShift_t GlobalOp::getBeltShift() {
  return m_instance->getBeltShift();
}

Carriage_t GlobalOp::getCarriage() {
  return m_instance->getCarriage();
}

Direction_t GlobalOp::getDirection() {
  return m_instance->getDirection();
}

Direction_t GlobalOp::getHallActive() {
  return m_instance->getHallActive();
}

uint8_t GlobalOp::getPosition() {
  return m_instance->getPosition();
}
