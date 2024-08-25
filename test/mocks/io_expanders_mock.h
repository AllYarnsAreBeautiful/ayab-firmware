/*!
 * \file io_expanders_mock.h
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

#ifndef IO_EXPANDERS_MOCK_H_
#define IO_EXPANDERS_MOCK_H_

#include <Wire.h>
#include <array>

/*! \brief Simulate I2C I/O expanders
 *
 * This class connects to arduino-mock's I2C (Wire) support and
 * intercepts writes to the simulated I2C bus, emulating a pair of
 * MCP23008 I/O expanders.
 *
 * Of the MCP23008 protocol, only the bare minimum necessary to support
 * GPIO writes is implemented.
 *
 * The current GPIO state can be retrieved at any time by calling
 * gpioState(). The result is the state of the 16 digital outputs,
 * in the order that they are normally connected to the knitting
 * machine's solenoids, i.e. starting from the leftmost solenoid.
 *
 * Note that a KH270 machine only has 12 solenoids, but since this
 * class simulates the I/O expanders, that always have 16 outputs,
 * that fact is not relevant here.
 *
 * Creating an instance of this class takes over WireMock, no expectations
 * should be setup on it from outside until that instance is destroyed
 * and releaseWireMock is called.
 */
class IOExpandersMock {
public:
  IOExpandersMock(WireMock *wireMock);

  std::array<bool, 16> gpioState();

private:
  uint8_t i2c_address = 0, i2c_byteIndex = 0, i2c_register = 0;
  uint8_t highByte = 0, lowByte = 0;

  void beginTransmission(uint8_t address);

  uint8_t write(uint8_t data);
};

#endif // IO_EXPANDERS_MOCK_H_
