/*!`
 * \file op_mock.cpp
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

#include <op.h>
#include <op_mock.h>

static OpMock *gOpMock = nullptr;

OpMock *opMockInstance() {
  if (!gOpMock) {
    gOpMock = new OpMock();
  }
  return gOpMock;
}

void releaseOpMock() {
  if (gOpMock) {
    delete gOpMock;
    gOpMock = nullptr;
  }
}

void Op::init() {
  assert(gOpMock != nullptr);
  gOpMock->init();
}

OpState_t Op::getState() {
  assert(gOpMock != nullptr);
  return gOpMock->getState();
}

void Op::setState(OpState_t state) {
  assert(gOpMock != nullptr);
  gOpMock->setState(state);
}

void Op::update() {
  assert(gOpMock != nullptr);
  gOpMock->update();
}

void Op::cacheEncoders() {
  assert(gOpMock != nullptr);
  gOpMock->cacheEncoders();
}
