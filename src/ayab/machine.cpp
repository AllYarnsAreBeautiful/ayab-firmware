/*!`
 * \file machine.cpp
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

#include "machine.h"

Machine_t Machine::getMachineType() const {
  return m_machineType;
}

void Machine::setMachineType(Machine_t machineType) {
  m_machineType = machineType;
}

uint8_t Machine::numNeedles() const {
  if (m_machineType == Kh270) {
    return 114U;
  }
  return 200U;
}

uint8_t Machine::lenLineBuffer() const {
  if (m_machineType == Kh270) {
    // KH270 has 114 needles = 15 bytes
    return 15U;
  }
  // Other machines have 200 needles = 25 bytes
  return 25U;
}

uint8_t Machine::endOfLineOffsetL() const {
  return 12U;
}

uint8_t Machine::endOfLineOffsetR() const {
  return 12U;
}

uint8_t Machine::startOffsetLUT(Direction_t direction, Carriage_t carriage) const {
  return m_startOffsetLUT[direction][carriage];
}
