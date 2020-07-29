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
 *    Modified Work Copyright 2020 Sturla Lange, Tom Price
 *    http://ayab-knitting.com
 */

#include <encoders_mock.h>
#include <encoders.h>

static EncodersMock *gEncodersMock = NULL;
EncodersMock *encodersMockInstance() {
  if (!gEncodersMock) {
    gEncodersMock = new EncodersMock();
  }
  return gEncodersMock;
}

void releaseEncodersMock() {
  if (gEncodersMock) {
    delete gEncodersMock;
    gEncodersMock = NULL;
  }
}

void Encoders::encA_interrupt() {
  assert(gEncodersMock != NULL);
  gEncodersMock->encA_interrupt();
}
uint8_t Encoders::getPosition() const {
  assert(gEncodersMock != NULL);
  return gEncodersMock->getPosition();
}
Beltshift_t Encoders::getBeltshift() {
  assert(gEncodersMock != NULL);
  return gEncodersMock->getBeltshift();
}
Direction_t Encoders::getDirection() {
  assert(gEncodersMock != NULL);
  return gEncodersMock->getDirection();
}
Direction_t Encoders::getHallActive() {
  assert(gEncodersMock != NULL);
  return gEncodersMock->getHallActive();
}
Carriage_t Encoders::getCarriage() {
  assert(gEncodersMock != NULL);
  return gEncodersMock->getCarriage();
}
uint16_t Encoders::getHallValue(Direction_t dir) {
  assert(gEncodersMock != NULL);
  return gEncodersMock->getHallValue(dir);
}
