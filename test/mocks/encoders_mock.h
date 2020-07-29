/*!`
 * \file encoders_mock.h
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

#ifndef ENCODERS_MOCK_H_
#define ENCODERS_MOCK_H_

#include <encoders.h>
#include <gmock/gmock.h>

class EncodersMock {
public:
  MOCK_METHOD0(encA_interrupt, void());
  MOCK_METHOD0(getPosition, uint8_t());
  MOCK_METHOD0(getBeltshift, Beltshift_t());
  MOCK_METHOD0(getDirection, Direction_t());
  MOCK_METHOD0(getHallActive, Direction_t());
  MOCK_METHOD0(getCarriage, Carriage_t());
  MOCK_METHOD1(getHallValue, uint16_t(Direction_t));
};

EncodersMock *encodersMockInstance();
void releaseEncodersMock();

#endif  // ENCODERS_MOCK_H_
