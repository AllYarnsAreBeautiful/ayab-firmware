/*!`
 * \file test_solenoids.cpp
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

#include <gtest/gtest.h>

#include <solenoids.h>
#ifdef HARD_I2C
#include <Wire.h>
#endif

using ::testing::Return;

extern Solenoids *solenoids;

class SolenoidsTest : public ::testing::Test {
protected:
  void SetUp() override {
    arduinoMock = arduinoMockInstance();
#ifdef HARD_I2C
    wireMock = WireMockInstance();
#endif
  }

  void TearDown() override {
    releaseArduinoMock();
  }

  ArduinoMock *arduinoMock;
#ifdef HARD_I2C
  WireMock *wireMock;
#endif
};

TEST_F(SolenoidsTest, test_construct) {
}

TEST_F(SolenoidsTest, test_init) {
  solenoids->init();
}

TEST_F(SolenoidsTest, test_setSolenoid1) {
  solenoids->setSolenoids(0);
  ASSERT_TRUE(solenoids->solenoidState == 0U);
  solenoids->setSolenoid(0, true);
  ASSERT_TRUE(solenoids->solenoidState == 1U);
}

TEST_F(SolenoidsTest, test_setSolenoid2) {
  solenoids->setSolenoids(0);
  ASSERT_TRUE(solenoids->solenoidState == 0U);
  solenoids->setSolenoids(0);
  ASSERT_TRUE(solenoids->solenoidState == 0U);
  solenoids->setSolenoid(0, false);
  ASSERT_TRUE(solenoids->solenoidState == 0U);
}

TEST_F(SolenoidsTest, test_setSolenoid3) {
  solenoids->setSolenoids(0x8000);
  ASSERT_TRUE(solenoids->solenoidState == 0x8000U);
  solenoids->setSolenoid(16, false);
  ASSERT_TRUE(solenoids->solenoidState == 0x8000U);
}
