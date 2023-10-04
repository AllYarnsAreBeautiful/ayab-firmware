/*!`
 * \file opError_mock.cpp
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

#include <opError_mock.h>

static OpErrorMock *gOpErrorMock = nullptr;

OpErrorMock *OpErrorMockInstance() {
  if (!gOpErrorMock) {
    gOpErrorMock = new OpErrorMock();
  }
  return gOpErrorMock;
}

void releaseOpErrorMock() {
  if (gOpErrorMock) {
    delete gOpErrorMock;
    gOpErrorMock = nullptr;
  }
}

OpState_t OpError::state() {
  assert(gOpErrorMock != nullptr);
  return gOpErrorMock->state();
}

void OpError::init() {
  assert(gOpErrorMock != nullptr);
  gOpErrorMock->init();
}

void OpError::begin() {
  assert(gOpErrorMock != nullptr);
  return gOpErrorMock->begin();
}

void OpError::update() {
  assert(gOpErrorMock != nullptr);
  gOpErrorMock->update();
}

void OpError::com(const uint8_t *buffer, size_t size) {
  assert(gOpErrorMock != nullptr);
  gOpErrorMock->com(buffer, size);
}

void OpError::end() {
  assert(gOpErrorMock != nullptr);
  gOpErrorMock->end();
}
