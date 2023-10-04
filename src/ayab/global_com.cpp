/*!
 * \file global_com.cpp
 * \brief Singleton class containing methods for serial communication.
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

#include "com.h"

// static member functions

void GlobalCom::init() {
  m_instance.init();
}

void GlobalCom::update() {
  m_instance.update();
}

uint8_t GlobalCom::CRC8(const uint8_t *buffer, size_t len) {
  return m_instance.CRC8(buffer, len);
}

void GlobalCom::send(uint8_t *payload, size_t length) {
  m_instance.send(payload, length);
}

void GlobalCom::sendMsg(API_t id, const char *msg) {
  m_instance.sendMsg(id, msg);
}

void GlobalCom::sendMsg(API_t id, char *msg) {
  m_instance.sendMsg(id, msg);
}

void GlobalCom::send_reqLine(const uint8_t lineNumber, Err_t error) {
  m_instance.send_reqLine(lineNumber, error);
}

void GlobalCom::send_indState(Err_t error) {
  m_instance.send_indState(error);
}

void GlobalCom::onPacketReceived(const uint8_t *buffer, size_t size) {
  m_instance.onPacketReceived(buffer, size);
}

void GlobalCom::h_reqInit(const uint8_t *buffer, size_t size) {
  m_instance.h_reqInit(buffer, size);
}

void GlobalCom::h_reqStart(const uint8_t *buffer, size_t size) {
  m_instance.h_reqStart(buffer, size);
}

void GlobalCom::h_cnfLine(const uint8_t *buffer, size_t size) {
  m_instance.h_cnfLine(buffer, size);
}

void GlobalCom::h_reqInfo() {
  m_instance.h_reqInfo();
}

void GlobalCom::h_reqTest() {
  m_instance.h_reqTest();
}

void GlobalCom::h_quitCmd() {
  m_instance.h_quitCmd();
}

void GlobalCom::h_unrecognized() {
  m_instance.h_unrecognized();
}
