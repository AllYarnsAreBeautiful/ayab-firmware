/*!
 * \file solenoids.cpp
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
 *    Modified Work Copyright 2020 Sturla Lange
 *    http://ayab-knitting.com
 */

#include "solenoids.h"

/*!
 * Initialize I2C connection for solenoids.
 */
void Solenoids::init() {
#ifdef HARD_I2C
  mcp_0.begin(I2Caddr_sol1_8);
  mcp_1.begin(I2Caddr_sol9_16);

  for (int i = 0; i < (SOLENOIDS_NUM / 2); i++) {
    mcp_0.pinMode(i, OUTPUT);
    mcp_1.pinMode(i, OUTPUT);
  }
#endif
  // No Action needed for SOFT_I2C
}

/*!
 * Set the state of a solenoid.
 *
 * \param solenoid The solenoid nr. to set. 0 to 15.
 * \param state The state to set the solenoid to.
 */
void Solenoids::setSolenoid(uint8_t solenoid, bool state) {
  if (solenoid > (SOLENOIDS_NUM - 1)) {
    // Only 16 solenoids (zero-indexed).
    return;
  }

  if (state) {
    bitSet(solenoidState, solenoid);
  } else {
    bitClear(solenoidState, solenoid);
  }

  // TODO(Who?): Optimize to act only when there is an actual change of state
  write(solenoidState);
}

/*!
 * Set the state of all the solenoids.
 *
 * \param state Two bytes describing the state of the solenoids,
 * one bit per solenoid.
 */
void Solenoids::setSolenoids(uint16_t state) {
  solenoidState = state;
  write(state);
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
void Solenoids::write(uint16_t newState) {
#ifdef HARD_I2C
  mcp_0.writeGPIO(lowByte(newState));
  mcp_1.writeGPIO(highByte(newState));
#elif defined SOFT_I2C
  SoftI2C.beginTransmission(I2Caddr_sol1_8 | SOLENOIDS_I2C_ADDRESS_MASK);
  SoftI2C.send(lowByte(newState));
  SoftI2C.endTransmission();
  SoftI2C.beginTransmission(I2Caddr_sol9_16 | SOLENOIDS_I2C_ADDRESS_MASK);
  SoftI2C.send(highByte(newState));
  SoftI2C.endTransmission();
#endif
}
