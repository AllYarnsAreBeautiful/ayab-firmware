 /*!
 * \file solenoids.cpp
 * \brief Class containing methods that control the needles
 *    via solenoids connected to IO expanders on the device.
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

#include "solenoids.h"

// Minimal required constants from the MCP23008 datasheet:
// - fixed I2C address bits
#define MCP23008_ADDRESS 0x20
// - used register addresses
#define MCP23008_IODIR 0x00
#define MCP23008_OLAT 0x0A

/*!
 * \brief Initialize I2C connection for solenoids.
 */
void Solenoids::init() {
  Wire.begin();

  // Default to all solenoids ON
  // This mimicks the behavior of Brother electronics and is done
  // to avoid a "clunking" sound that occurs if solenoids are turned
  // OFF after having been ON while the carriage was moving.
  solenoidState = 0xffffU;
  writeGPIO(solenoidState);

  // Configure all MCP23008 pins as outputs
  writeRegister(I2Caddr_sol1_8, MCP23008_IODIR, 0x00);
  writeRegister(I2Caddr_sol9_16, MCP23008_IODIR, 0x00);
}

/*!
 * \brief Set the state of a solenoid.
 *
 * \param solenoid The solenoid number to set (0 to 15).
 * \param state The state to set the solenoid to.
 */
void Solenoids::setSolenoid(uint8_t solenoid, bool state) {
  if (solenoid > (SOLENOID_BUFFER_SIZE - 1)) {
    // Solenoid buffer is 16 bits (zero-indexed).
    return;
  }

  uint16_t oldState = solenoidState;
  if (state) {
    bitSet(solenoidState, solenoid);
  } else {
    bitClear(solenoidState, solenoid);
  }
  if (oldState != solenoidState) {
#ifndef AYAB_TESTS
    writeGPIO(solenoidState);
#endif
  }
}

/*!
 * \brief Set the state of all the solenoids.
 *
 * \param state Two bytes describing the state of the solenoids,
 * one bit per solenoid.
 */
void Solenoids::setSolenoids(uint16_t state) {
  if (state != solenoidState) {
    solenoidState = state;
#ifndef AYAB_TESTS
    writeGPIO(state);
#endif
  }
}

/*
 * Private Methods
 */

/*!
 * Write to the I2C port expanders.
 *
 * Low level function, mapping to actual wiring
 * is done here.
 *
 * \param newState Two bytes describing the state of the solenoids,
 * one bit per solenoid.
 */
// GCOVR_EXCL_START
void Solenoids::writeGPIO(uint16_t newState) {
  writeRegister(I2Caddr_sol1_8, MCP23008_OLAT, lowByte(newState));
  writeRegister(I2Caddr_sol9_16, MCP23008_OLAT, highByte(newState));
}

/*!
 * Write to an MCP23008 register via I2C
 *
 * \param i2caddr Address of the I/O expander, only the 3 lowest bits are
 * used, mapping to the A0..A2 address pins on MCP23008
 * \param reg Register address (see MCP23008 datasheet)
 * \param value Value to set the register to
 */
void Solenoids::writeRegister(uint8_t i2caddr, uint8_t reg, uint8_t value) {
  Wire.beginTransmission(MCP23008_ADDRESS | (i2caddr & 7));
  Wire.write(reg);
  Wire.write(value);
  Wire.endTransmission();
}
// GCOVR_EXCL_STOP
