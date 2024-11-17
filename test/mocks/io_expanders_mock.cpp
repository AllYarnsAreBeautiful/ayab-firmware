/*!
 * \file io_expanders_mock.cpp
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

#include "io_expanders_mock.h"

#include <Adafruit_MCP23008.h>
#include <gmock/gmock.h>

#include <array>

using namespace ::testing;

IOExpandersMock::IOExpandersMock(WireMock *wireMock) {
  EXPECT_CALL(*wireMock, beginTransmission(_))
      .Times(AnyNumber())
      .WillRepeatedly(Invoke(this, &IOExpandersMock::beginTransmission));
  EXPECT_CALL(*wireMock, write(An<uint8_t>()))
      .Times(AnyNumber())
      .WillRepeatedly(Invoke(this, &IOExpandersMock::write));

  // We're not interested in calls to the following methods, but they
  // happen and if we don't set up expectations for them GoogleMock will
  // emit warnings.
  EXPECT_CALL(*wireMock, begin()).Times(AnyNumber());
  EXPECT_CALL(*wireMock, endTransmission()).Times(AnyNumber());
  EXPECT_CALL(*wireMock, requestFrom(_, _)).Times(AnyNumber());
  EXPECT_CALL(*wireMock, read).Times(AnyNumber());
}

std::array<bool, 16> IOExpandersMock::gpioState() {
  std::array<bool, 16> result;
  for (int i = 0; i < 8; i++) {
    result[i] = lowByte & (1 << i);
    result[i + 8] = highByte & (1 << i);
  }
  return result;
}

void IOExpandersMock::beginTransmission(uint8_t address) {
  i2c_address = address;
  i2c_byteIndex = i2c_register = 0;
}

uint8_t IOExpandersMock::write(uint8_t data) {
  switch (i2c_byteIndex++) {
  case 0:
    i2c_register = data;
    break;
  case 1:
    switch (i2c_register) {
    case MCP23008_GPIO:
    case MCP23008_OLAT:
      switch (i2c_address & ~MCP23008_ADDRESS) {
      case 0:
        lowByte = data;
        break;
      case 1:
        highByte = data;
        break;
      }
      break;
    }
  }
  return 0;
}