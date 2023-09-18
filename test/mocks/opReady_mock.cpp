/*!`
 * \file op_ready_mock.cpp
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

#include <op_ready_mock.h>

static OpReadyMock *gOpReadyMock = nullptr;

OpReadyMock *OpReadyMockInstance() {
  if (!gOpReadyMock) {
    gOpReadyMock = new OpReadyMock();
  }
  return gOpReadyMock;
}

void releaseOpReadyMock() {
  if (gOpReadyMock) {
    delete gOpReadyMock;
    gOpReadyMock = nullptr;
  }
}

void OpReady::init() {
  assert(gOpReadyMock != nullptr);
  gOpReadyMock->init();
}

Err_t OpReady::begin() {
  assert(gOpReadyMock != nullptr);
  return gOpReadyMock->begin();
}

void OpReady::update() {
  assert(gOpReadyMock != nullptr);
  gOpReadyMock->update();
}

void OpReady::com(const uint8_t *buffer, size_t size) {
  assert(gOpReadyMock != nullptr);
  gOpReadyMock->com(buffer, size);
}

void OpReady::end() {
  assert(gOpReadyMock != nullptr);
  gOpReadyMock->end();
}
