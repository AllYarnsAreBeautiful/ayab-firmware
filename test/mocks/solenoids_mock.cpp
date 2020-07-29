/*!`
 * \file solenoids_mock.cpp
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

#include <solenoids_mock.h>

static SolenoidsMock *gSolenoidsMock = NULL;
SolenoidsMock *solenoidsMockInstance() {
  if (!gSolenoidsMock) {
    gSolenoidsMock = new SolenoidsMock();
  }
  return gSolenoidsMock;
}

void releaseSolenoidsMock() {
  if (gSolenoidsMock) {
    delete gSolenoidsMock;
    gSolenoidsMock = NULL;
  }
}

void Solenoids::init(void) {
  assert(gSolenoidsMock != NULL);
  gSolenoidsMock->init();
}
void Solenoids::setSolenoid(uint8_t solenoid, bool state) {
  assert(gSolenoidsMock != NULL);
  gSolenoidsMock->setSolenoid(solenoid, state);
}
void Solenoids::setSolenoids(uint16_t state) {
  assert(gSolenoidsMock != NULL);
  gSolenoidsMock->setSolenoids(state);
}
