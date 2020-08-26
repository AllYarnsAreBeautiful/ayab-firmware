/*!`
 * \file com_mock.cpp
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

#include <com.h>
#include <com_mock.h>

static ComMock *gComMock = NULL;

ComMock *comMockInstance() {
  if (!gComMock) {
    gComMock = new ComMock();
  }
  return gComMock;
}

void releaseComMock() {
  if (gComMock) {
    delete gComMock;
    gComMock = NULL;
  }
}

void Com::init() {
  assert(gComMock != nullptr);
  gComMock->init();
}

void Com::update() {
  assert(gComMock != nullptr);
  gComMock->update();
}

void Com::send(uint8_t *payload, size_t length) {
  assert(gComMock != nullptr);
  gComMock->send(payload, length);
}

void Com::sendMsg(AYAB_API_t id, const char *msg) {
  assert(gComMock != nullptr);
  gComMock->sendMsg(id, msg);
}

void Com::sendMsg(AYAB_API_t id, char *msg) {
  assert(gComMock != nullptr);
  gComMock->sendMsg(id, msg);
}

void Com::send_reqLine(const uint8_t lineNumber, Err_t error) {
  assert(gComMock != nullptr);
  gComMock->send_reqLine(lineNumber, error);
}

void Com::send_indState(Carriage_t carriage, uint8_t position,
                        const uint8_t initState) {
  assert(gComMock != nullptr);
  gComMock->send_indState(carriage, position, initState);
}

void Com::onPacketReceived(const uint8_t *buffer, size_t size) {
  assert(gComMock != nullptr);
  gComMock->onPacketReceived(buffer, size);
}
