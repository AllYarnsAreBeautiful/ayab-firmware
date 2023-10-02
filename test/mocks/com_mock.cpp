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
 *    Modified Work Copyright 2020-3 Sturla Lange, Tom Price
 *    http://ayab-knitting.com
 */

#include <com.h>
#include <com_mock.h>

static ComMock *gComMock = nullptr;

ComMock *comMockInstance() {
  if (!gComMock) {
    gComMock = new ComMock();
  }
  return gComMock;
}

void releaseComMock() {
  if (gComMock) {
    delete gComMock;
    gComMock = nullptr;
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

uint8_t Com::CRC8(const uint8_t *buffer, size_t len) const {
  assert(gComMock != nullptr);
  return gComMock->CRC8(buffer, len);
}

void Com::send(uint8_t *payload, size_t length) const {
  assert(gComMock != nullptr);
  gComMock->send(payload, length);
}

void Com::sendMsg(API_t id, const char *msg) {
  assert(gComMock != nullptr);
  gComMock->sendMsg(id, msg);
}

void Com::sendMsg(API_t id, char *msg) {
  assert(gComMock != nullptr);
  gComMock->sendMsg(id, msg);
}

void Com::send_reqLine(const uint8_t lineNumber, Err_t error) const {
  assert(gComMock != nullptr);
  gComMock->send_reqLine(lineNumber, error);
}

void Com::send_indState(Err_t error) const {
  assert(gComMock != nullptr);
  gComMock->send_indState(error);
}

void Com::onPacketReceived(const uint8_t *buffer, size_t size) {
  assert(gComMock != nullptr);
  gComMock->onPacketReceived(buffer, size);
}

void Com::h_reqInit(const uint8_t (&buffer)[3]) {
  assert(gComMock != nullptr);
  gComMock->h_reqInit(buffer);
}

void Com::h_reqStart(const uint8_t *buffer, size_t size) {
  assert(gComMock != nullptr);
  gComMock->h_reqStart(buffer, size);
}

void Com::h_cnfLine(const uint8_t *buffer, size_t size) {
  assert(gComMock != nullptr);
  gComMock->h_cnfLine(buffer, size);
}

void Com::h_reqInfo() const {
  assert(gComMock != nullptr);
  gComMock->h_reqInfo();
}

void Com::h_reqTest() const {
  assert(gComMock != nullptr);
  gComMock->h_reqTest();
}

void Com::h_quitCmd() const {
  assert(gComMock != nullptr);
  gComMock->h_quitCmd();
}

void Com::h_unrecognized() const {
  assert(gComMock != nullptr);
  gComMock->h_unrecognized();
}
