/*!`
 * \file knitter_mock.h
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

#ifndef KNITTER_MOCK_H_
#define KNITTER_MOCK_H_

#include <gmock/gmock.h>
#include <knitter.h>

class KnitterMock : public KnitterInterface {
public:
  MOCK_METHOD0(init, void());
  MOCK_METHOD0(setUpInterrupt, void());
  MOCK_METHOD0(isr, void());
  MOCK_METHOD4(startKnitting, Err_t(uint8_t startNeedle,
                                    uint8_t stopNeedle, uint8_t *pattern_start,
                                    bool continuousReportingEnabled));
  MOCK_METHOD1(initMachine, Err_t(Machine_t machineType));
  MOCK_METHOD0(encodePosition, void());
  MOCK_METHOD0(isReady, bool());
  MOCK_METHOD0(knit, void());
  MOCK_METHOD1(indState, void(Err_t error));
  MOCK_METHOD1(getStartOffset, uint8_t(const Direction_t direction));
  MOCK_METHOD0(getMachineType, Machine_t());
  MOCK_METHOD1(setNextLine, bool(uint8_t lineNumber));
  MOCK_METHOD0(setLastLine, void());
  MOCK_METHOD1(setMachineType, void(Machine_t));
};

KnitterMock *knitterMockInstance();
void releaseKnitterMock();

#endif // KNITTER_MOCK_H_
