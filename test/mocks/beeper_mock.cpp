/*!`
 * \file beeper_mock.cpp
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

#include <beeper_mock.h>

static BeeperMock *gBeeperMock = nullptr;
BeeperMock *beeperMockInstance() {
  if (!gBeeperMock) {
    gBeeperMock = new BeeperMock();
  }
  return gBeeperMock;
}

void releaseBeeperMock() {
  if (gBeeperMock) {
    delete gBeeperMock;
    gBeeperMock = nullptr;
  }
}

void Beeper::init() {
  assert(gBeeperMock != nullptr);
  gBeeperMock->init();
}

BeepState Beeper::getState() {
  assert(gBeeperMock != nullptr);
  return gBeeperMock->getState();
}

void Beeper::ready() {
  assert(gBeeperMock != nullptr);
  gBeeperMock->ready();
}

void Beeper::finishedLine() {
  assert(gBeeperMock != nullptr);
  gBeeperMock->finishedLine();
}

void Beeper::endWork() {
  assert(gBeeperMock != nullptr);
  gBeeperMock->endWork();
}

void Beeper::schedule() {
  assert(gBeeperMock != nullptr);
  gBeeperMock->schedule();
}
