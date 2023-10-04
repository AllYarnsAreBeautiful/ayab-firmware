/*!`
 * \file controller_mock.cpp
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
#include <controller_mock.h>

static ControllerMock *gControllerMock = nullptr;

ControllerMock *controllerMockInstance() {
  if (!gControllerMock) {
    gControllerMock = new ControllerMock();
  }
  return gControllerMock;
}

void releaseControllerMock() {
  if (gControllerMock) {
    delete gControllerMock;
    gControllerMock = nullptr;
  }
}

void Controller::init() {
  assert(gControllerMock != nullptr);
  gControllerMock->init();
}

void Controller::update() {
  assert(gControllerMock != nullptr);
  gControllerMock->update();
}

void Controller::com(const uint8_t *buffer, size_t size) const {
  assert(gControllerMock != nullptr);
  return gControllerMock->com(buffer, size);
}

void Controller::cacheEncoders() {
  assert(gControllerMock != nullptr);
  gControllerMock->cacheEncoders();
}

void Controller::setState(OpInterface *state) {
  assert(gControllerMock != nullptr);
  gControllerMock->setState(state);
}

OpInterface *Controller::getState() const {
  assert(gControllerMock != nullptr);
  return gControllerMock->getState();
}

void Controller::setMachineType(Machine_t machineType) {
  assert(gControllerMock != nullptr);
  gControllerMock->setMachineType(machineType);
}

Machine_t Controller::getMachineType() const {
  assert(gControllerMock != nullptr);
  return gControllerMock->getMachineType();
}

BeltShift_t Controller::getBeltShift() const {
  assert(gControllerMock != nullptr);
  return gControllerMock->getBeltShift();
}

Carriage_t Controller::getCarriage() const {
  assert(gControllerMock != nullptr);
  return gControllerMock->getCarriage();
}

Direction_t Controller::getDirection() const {
  assert(gControllerMock != nullptr);
  return gControllerMock->getDirection();
}

Direction_t Controller::getHallActive() const {
  assert(gControllerMock != nullptr);
  return gControllerMock->getHallActive();
}

uint8_t Controller::getPosition() const {
  assert(gControllerMock != nullptr);
  return gControllerMock->getPosition();
}
