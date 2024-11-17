/*!
 * \file knitting_machine_adapter.cpp
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
#include "knitting_machine_adapter.h"

#include <gmock/gmock.h>

#include "knitting_machine.h"

#include "board.h"

using namespace ::testing;

KnittingMachineAdapter::KnittingMachineAdapter(KnittingMachine &km,
                                               ArduinoMock &arduinoMock,
                                               Flags flags)
    : m_km(km), m_flags(flags) {
  EXPECT_CALL(arduinoMock, digitalRead(ENC_PIN_A))
      .Times(AnyNumber())
      .WillRepeatedly(
          Invoke([&] { return m_km.getEncoderOutput1() ? HIGH : LOW; }));

  EXPECT_CALL(arduinoMock, digitalRead(ENC_PIN_B))
      .Times(AnyNumber())
      .WillRepeatedly(
          Invoke([&] { return m_km.getEncoderOutput2() ? HIGH : LOW; }));

  EXPECT_CALL(arduinoMock, analogRead(EOL_PIN_L))
      .Times(AnyNumber())
      .WillRepeatedly(Invoke(
          [&] { return m_km.getLeftPositionSensorVoltage() * 1023 / 5; }));

  EXPECT_CALL(arduinoMock, digitalRead(ENC_PIN_C))
      .Times(AnyNumber())
      .WillRepeatedly(Invoke([&] { return m_km.getBeltPhase() ? HIGH : LOW; }));

  EXPECT_CALL(arduinoMock, analogRead(EOL_PIN_R))
      .Times(AnyNumber())
      .WillRepeatedly(Invoke([&] {
        if (m_flags & DigitalRightSensor) {
          // On the KH-910 EOL_PIN_R is plugged into the K digital signal
          return m_km.getRightPositionSensorKSignal() * 1023 / 5;
        } else {
          return m_km.getRightPositionSensorVoltage() * 1023 / 5;
        }
      }));
}