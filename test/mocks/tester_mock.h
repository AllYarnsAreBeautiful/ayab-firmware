/*!`
 * \file tester_mock.h
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

#ifndef TESTER_MOCK_H_
#define TESTER_MOCK_H_

#include <gmock/gmock.h>
#include <tester.h>

class TesterMock : public TesterInterface {
public:
  MOCK_METHOD1(startTest, Err_t(Machine_t machineType));
  MOCK_METHOD0(loop, void());
  MOCK_METHOD0(helpCmd, void());
  MOCK_METHOD0(sendCmd, void());
  MOCK_METHOD0(beepCmd, void());
  MOCK_METHOD2(setSingleCmd, void(const uint8_t *, size_t));
  MOCK_METHOD2(setAllCmd, void(const uint8_t *, size_t));
  MOCK_METHOD0(readEOLsensorsCmd, void());
  MOCK_METHOD0(readEncodersCmd, void());
  MOCK_METHOD0(autoReadCmd, void());
  MOCK_METHOD0(autoTestCmd, void());
  MOCK_METHOD0(stopCmd, void());
  MOCK_METHOD0(quitCmd, void());
  MOCK_METHOD0(encoderChange, void());
};

TesterMock *testerMockInstance();
void releaseTesterMock();

#endif // TESTER_MOCK_H_
