/*!`
 * \file opInit_mock.cpp
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

#include <opInit_mock.h>

static OpInitMock *gOpInitMock = nullptr;

OpInitMock *OpInitMockInstance() {
  if (!gOpInitMock) {
    gOpInitMock = new OpInitMock();
  }
  return gOpInitMock;
}

void releaseOpInitMock() {
  if (gOpInitMock) {
    delete gOpInitMock;
    gOpInitMock = nullptr;
  }
}

void OpInit::init() {
  assert(gOpInitMock != nullptr);
  gOpInitMock->init();
}

Err_t OpInit::begin() {
  assert(gOpInitMock != nullptr);
  return gOpInitMock->begin();
}

void OpInit::update() {
  assert(gOpInitMock != nullptr);
  gOpInitMock->update();
}

void OpInit::com(const uint8_t *buffer, size_t size) {
  assert(gOpInitMock != nullptr);
  gOpInitMock->com(buffer, size);
}

void OpInit::end() {
  assert(gOpInitMock != nullptr);
  gOpInitMock->end();
}
