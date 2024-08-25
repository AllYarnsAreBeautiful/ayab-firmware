/*!
 * \file knitting_machine_adapter.h
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
 *    http://ayab-knitting.com
 */
#ifndef KNITTING_MACHINE_ADAPTER_H_
#define KNITTING_MACHINE_ADAPTER_H_

#include <Arduino.h>

#include "knitting_machine.h"

/**
 * This class connects a KnittingMachine (which does not know anything about
 * testing/mocking, or the AYAB code) to ArduinoMock, so that calls to
 * the relevant Arduino functions (e.g. `digitalRead`) made from the AYAB
 * code will retrieve or set information on the KnittingMachine instance.
 *
 * Note that this class makes no attempt at clean-up (because the gMock
 * API makes this basically impossible), so once it is destroyed,
 * no call should be made to ArduinoMock until releaseArduinoMock()
 * has been called.
 */
class KnittingMachineAdapter {
public:
  enum Flags {
    Default = 0,
    DigitalRightSensor = 1,
  };

  KnittingMachineAdapter(KnittingMachine &km, ArduinoMock &arduinoMock,
                         Flags flags = Default);

private:
  KnittingMachine &m_km;
  Flags m_flags;
};

#endif // KNITTING_MACHINE_ADAPTER_H_