/*!`
 * \file fsm_mock.h
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

#ifndef FSM_MOCK_H_
#define FSM_MOCK_H_

#include <gmock/gmock.h>

#include <encoders.h>
#include <fsm.h>

class FsmMock : public FsmInterface {
public:
  MOCK_METHOD0(init, void());
  MOCK_METHOD0(update, void());
  MOCK_METHOD0(cacheEncoders, void());
  MOCK_METHOD1(setState, void(OpInterface *state));
  MOCK_METHOD0(getState, OpInterface*());
  MOCK_METHOD1(setMachineType, void(Machine_t machineType));
  MOCK_METHOD0(getMachineType, Machine_t());
  MOCK_METHOD0(getBeltShift, BeltShift_t());
  MOCK_METHOD0(getCarriage, Carriage_t());
  MOCK_METHOD0(getDirection, Direction_t());
  MOCK_METHOD0(getHallActive, Direction_t());
  MOCK_METHOD0(getPosition, uint8_t());
};

FsmMock *fmsMockInstance();
void releaseFsmMock();

#endif // FSM_MOCK_H_
