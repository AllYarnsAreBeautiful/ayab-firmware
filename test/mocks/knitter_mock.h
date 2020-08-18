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

class KnitterMock {
public:
  MOCK_METHOD5(startOperation, bool(Machine_t machineType, uint8_t startNeedle,
                                    uint8_t stopNeedle, uint8_t *pattern_start,
                                    bool continuousReportingEnabled));
  MOCK_METHOD1(startTest, bool(Machine_t));
  MOCK_METHOD1(setNextLine, bool(uint8_t lineNumber));
  MOCK_METHOD0(setLastLine, void());
  MOCK_METHOD2(send, void(uint8_t *payload, size_t length));
  MOCK_METHOD2(sendMsg, void(AYAB_API_t id, const char *msg));
  MOCK_METHOD2(sendMsg, void(AYAB_API_t id, char *msg));
  MOCK_METHOD2(onPacketReceived, void(const uint8_t *buffer, size_t size));
  MOCK_CONST_METHOD0(getMachineType, Machine_t());
  MOCK_METHOD1(setMachineType, void(Machine_t));
  MOCK_CONST_METHOD1(getStartOffset, uint8_t(const Direction_t direction));
  MOCK_METHOD1(setState, void(OpState_t));
  MOCK_METHOD1(setSolenoids, void(uint16_t state));
  MOCK_METHOD2(setSolenoid, void(uint8_t solenoid, uint8_t state));
  MOCK_METHOD1(setQuitFlag, void(bool flag));
  MOCK_METHOD0(setUpInterrupt, void());
};

KnitterMock *knitterMockInstance();
void releaseKnitterMock();

#endif // KNITTER_MOCK_H_
