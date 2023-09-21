/*!`
 * \file encoders_mock.cpp
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
 *    Modified Work Copyright 2020-3 Sturla Lange, Tom Price
 *    http://ayab-knitting.com
 */

#include <encoders.h>
#include <encoders_mock.h>

static EncodersMock *gEncodersMock = nullptr;
EncodersMock *encodersMockInstance() {
  if (!gEncodersMock) {
    gEncodersMock = new EncodersMock();
  }
  return gEncodersMock;
}

void releaseEncodersMock() {
  if (gEncodersMock) {
    delete gEncodersMock;
    gEncodersMock = nullptr;
  }
}

void Encoders::init(Machine_t machineType) {
  assert(gEncodersMock != nullptr);
  return gEncodersMock->init(machineType);
}

void Encoders::encA_interrupt() {
  assert(gEncodersMock != nullptr);
  gEncodersMock->encA_interrupt();
}

uint16_t Encoders::getHallValue(Direction_t dir) {
  assert(gEncodersMock != nullptr);
  return gEncodersMock->getHallValue(dir);
}

BeltShift_t Encoders::getBeltShift() {
  assert(gEncodersMock != nullptr);
  return gEncodersMock->getBeltShift();
}

Direction_t Encoders::getDirection() {
  assert(gEncodersMock != nullptr);
  return gEncodersMock->getDirection();
}

Carriage_t Encoders::getCarriage() {
  assert(gEncodersMock != nullptr);
  return gEncodersMock->getCarriage();
}

Machine_t Encoders::getMachineType() {
  assert(gEncodersMock != nullptr);
  return gEncodersMock->getMachineType();
}

Direction_t Encoders::getHallActive() {
  assert(gEncodersMock != nullptr);
  return gEncodersMock->getHallActive();
}

uint8_t Encoders::getPosition() {
  assert(gEncodersMock != nullptr);
  return gEncodersMock->getPosition();
}

