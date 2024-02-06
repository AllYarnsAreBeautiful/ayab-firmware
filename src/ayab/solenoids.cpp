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

/*!
 * \brief Initialize I2C connection for solenoids.
 */
void Solenoids::init() {
  #if defined(HAS_MCP23008)
  mcp_0.begin_I2C(I2Caddr_sol1_8);
  mcp_1.begin_I2C(I2Caddr_sol9_16);

  for (uint8_t i = 0; i < SOLENOID_BUFFER_SIZE / 2; i++) {
    mcp_0.pinMode(i, OUTPUT);
    mcp_1.pinMode(i, OUTPUT);
  }

  #elif defined(HAS_MCP23017)
  mcp.begin_I2C(MCP23017_ADDR_0);

  for (uint8_t i = 0; i < SOLENOID_BUFFER_SIZE; i++){
    mcp.pinMode(i, OUTPUT);
  }

  #endif
  solenoidState = 0x0000U;
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
    write(solenoidState);
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
    write(state);
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
void Solenoids::write(uint16_t newState) {
  (void)newState;
  #if defined(HAS_MCP23008)
  mcp_0.writeGPIO(lowByte(newState));
  mcp_1.writeGPIO(highByte(newState));

  #elif defined(HAS_MCP23017)
  // We need to shuffle the bits around due to hardware layout. 
  // GPA0..8 => solenoid 8-F
  // GPB0..8 => solenoid 7-0
  // Adafruit mapping: GPA0...8, GPB0..8 => 0..15
  uint16_t bankA = (newState >> 8); // map solenoids 8..F to 0..7 GPIO A

  uint16_t bankB = 0;
  for(uint16_t i = 0; i < 8; i++){
    // Need to reverse the bits of the upper byte (which is located in the lower byte)
    bankB |= (newState >> (7-i)) & 0x01;
    bankB << 1;
  }

  mcp.writeGPIOAB(bankA & bankB);

  #endif
}
// GCOVR_EXCL_STOP
