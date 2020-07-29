/*!`
 * \file machine_mock.h
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

#ifndef MACHINE_MOCK_H_
#define MACHINE_MOCK_H_

#include <machine.h>
#include <gmock/gmock.h>

class MachineMock : public Machine {
public:
  MOCK_METHOD0(getMachineType, Machine_t());
  MOCK_METHOD1(setMachineType, void(Machine_t machineType));
  MOCK_METHOD0(numNeedles, uint8_t());
  MOCK_METHOD0(lenLineBuffer, uint8_t());
  MOCK_METHOD0(endOfLineOffsetL, uint8_t());
  MOCK_METHOD0(endOfLineOffsetR, uint8_t());
  MOCK_METHOD2(startOffsetLUT, uint8_t(Direction_t direction, Carriage_t carriage));
};

#endif  // MACHINE_MOCK_H_
