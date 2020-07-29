/*!`
 * \file machine.h
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

#ifndef MACHINE_H_
#define MACHINE_H_

#include "encoders.h"

// Machine constants

constexpr uint8_t m_startOffsetLUT[NUM_DIRECTIONS][NUM_CARRIAGES] = {
    // NC,  K,  L,  G
    {0, 0, 0, 0},    // NoDirection
    {0, 40, 40, 8},  // Left
    {0, 16, 16, 32}, // Right
};

enum MachineType { Kh910, Kh930, Kh270, NO_MACHINE };
using Machine_t = enum MachineType;

class Machine {
public:
  virtual ~Machine() {};

  virtual Machine_t getMachineType() const;
  virtual void setMachineType(Machine_t machineType);

  virtual uint8_t numNeedles() const;
  virtual uint8_t lenLineBuffer() const;
  virtual uint8_t endOfLineOffsetL() const;
  virtual uint8_t endOfLineOffsetR() const;
  virtual uint8_t startOffsetLUT(Direction_t direction, Carriage_t carriage) const;

private:
  Machine_t m_machineType = NO_MACHINE;
};

#endif // MACHINE_H_
