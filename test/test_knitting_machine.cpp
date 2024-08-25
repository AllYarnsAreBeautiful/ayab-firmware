/*!
 * \file test_knitting_machine.cpp
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

/**
 * This test suite is meant to validate that the KnittingMachine class
 * faithfully reproduces the behavior of a real knitting machine.
 */

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <limits.h>

#include "knitting_machine.h"

using namespace ::testing;

TEST(KnittingMachine, EncoderOutputsMovingRight) {
  KnittingMachine km;

  ASSERT_EQ(false, km.getEncoderOutput1());
  ASSERT_EQ(true, km.getEncoderOutput2());

  km.moveBeltRight();

  ASSERT_EQ(true, km.getEncoderOutput1());
  ASSERT_EQ(true, km.getEncoderOutput2());

  km.moveBeltRight();

  ASSERT_EQ(true, km.getEncoderOutput1());
  ASSERT_EQ(false, km.getEncoderOutput2());

  km.moveBeltRight();

  ASSERT_EQ(false, km.getEncoderOutput1());
  ASSERT_EQ(false, km.getEncoderOutput2());

  km.moveBeltRight();

  ASSERT_EQ(false, km.getEncoderOutput1());
  ASSERT_EQ(true, km.getEncoderOutput2());
}

TEST(KnittingMachine, EncoderOutputsMovingLeft) {
  KnittingMachine km;

  ASSERT_EQ(false, km.getEncoderOutput1());
  ASSERT_EQ(true, km.getEncoderOutput2());

  km.moveBeltLeft();

  ASSERT_EQ(false, km.getEncoderOutput1());
  ASSERT_EQ(false, km.getEncoderOutput2());

  km.moveBeltLeft();

  ASSERT_EQ(true, km.getEncoderOutput1());
  ASSERT_EQ(false, km.getEncoderOutput2());

  km.moveBeltLeft();

  ASSERT_EQ(true, km.getEncoderOutput1());
  ASSERT_EQ(true, km.getEncoderOutput2());

  km.moveBeltLeft();

  ASSERT_EQ(false, km.getEncoderOutput1());
  ASSERT_EQ(true, km.getEncoderOutput2());
}

TEST(KnittingMachine, KCarriageDetectionOnTheLeft) {
  KnittingMachine km;

  km.putCarriageCenterInFrontOfNeedle(-100);
  km.addCarriageMagnet(0, true);

  ASSERT_THAT(km.getLeftPositionSensorVoltage(), AllOf(Gt(0.4), Lt(3.4)));

  km.putCarriageCenterInFrontOfNeedle(0);

  ASSERT_THAT(km.getLeftPositionSensorVoltage(), Gt(3.4));

  km.putCarriageCenterInFrontOfNeedle(5);

  ASSERT_THAT(km.getLeftPositionSensorVoltage(), AllOf(Gt(0.4), Lt(3.4)));
}

TEST(KnittingMachine, LCarriageDetectionOnTheLeft) {
  KnittingMachine km;

  km.addCarriageMagnet(0, false);

  km.putCarriageCenterInFrontOfNeedle(0);

  ASSERT_THAT(km.getLeftPositionSensorVoltage(), Lt(0.4));

  km.putCarriageCenterInFrontOfNeedle(5);

  ASSERT_THAT(km.getLeftPositionSensorVoltage(), AllOf(Gt(0.4), Lt(3.4)));
}

TEST(KnittingMachine, GCarriageDetectionOnTheLeft) {
  KnittingMachine km;

  km.addGCarriageMagnets();

  km.putCarriageCenterInFrontOfNeedle(-12);
  ASSERT_THAT(km.getLeftPositionSensorVoltage(), Lt(0.4));

  km.putCarriageCenterInFrontOfNeedle(-11);
  ASSERT_THAT(km.getLeftPositionSensorVoltage(), Gt(3.4));

  km.putCarriageCenterInFrontOfNeedle(0);
  ASSERT_THAT(km.getLeftPositionSensorVoltage(), AllOf(Gt(0.4), Lt(3.4)));

  km.putCarriageCenterInFrontOfNeedle(10);
  ASSERT_THAT(km.getLeftPositionSensorVoltage(), Gt(3.4));

  km.putCarriageCenterInFrontOfNeedle(11);
  ASSERT_THAT(km.getLeftPositionSensorVoltage(), Lt(0.4));
}

TEST(KnittingMachine, KCarriageDetectionOnTheRight) {
  KnittingMachine km;

  km.putCarriageCenterInFrontOfNeedle(-100);
  km.addCarriageMagnet(0, true);

  ASSERT_THAT(km.getRightPositionSensorVoltage(), AllOf(Gt(0.4), Lt(3.4)));
  ASSERT_THAT(km.getRightPositionSensorKSignal(), AllOf(Gt(1), Lt(4)));

  km.putCarriageCenterInFrontOfNeedle(200);

  ASSERT_THAT(km.getRightPositionSensorVoltage(), Gt(3.4));
  ASSERT_THAT(km.getRightPositionSensorKSignal(), Lt(0.4));

  km.putCarriageCenterInFrontOfNeedle(190);

  ASSERT_THAT(km.getRightPositionSensorVoltage(), AllOf(Gt(0.4), Lt(3.4)));
  ASSERT_THAT(km.getRightPositionSensorKSignal(), AllOf(Gt(1), Lt(4)));
}

TEST(KnittingMachine, MoveCarriageIncrementally) {
  KnittingMachine km;

  km.putCarriageCenterInFrontOfNeedle(-1);
  ASSERT_EQ(km.getCarriageCenterNeedle(), -1);

  // One step to the right isn't enough to change needle
  km.moveCarriageRight();
  ASSERT_EQ(km.getCarriageCenterNeedle(), -1);

  // Three more will do the trick
  km.moveCarriageRight();
  km.moveCarriageRight();
  km.moveCarriageRight();
  ASSERT_EQ(km.getCarriageCenterNeedle(), 0);

  // There's a helper to move until a position is reached
  int stepCount = 0;
  while (km.moveCarriageCenterTowardsNeedle(5)) {
    stepCount++;
  }

  ASSERT_EQ(km.getCarriageCenterNeedle(), 5);
  ASSERT_EQ(stepCount, 20);

  // You can go to the left as well
  while (km.moveCarriageCenterTowardsNeedle(0))
    ;
  ASSERT_EQ(km.getCarriageCenterNeedle(), 0);
}

TEST(KnittingMachine, BeltPhaseSignal) {
  KnittingMachine km;

  ASSERT_FALSE(km.getBeltPhase());

  for (int i = 0; i < 8 * 4; i++) {
    km.moveBeltLeft();
  }
  ASSERT_TRUE(km.getBeltPhase());

  for (int i = 0; i < 8 * 4; i++) {
    km.moveBeltRight();
  }
  ASSERT_FALSE(km.getBeltPhase());
}

TEST(KnittingMachine, CarriageMovesBeltOnlyWhenInSync) {
  KnittingMachine km;

  ASSERT_FALSE(km.getBeltPhase());

  // Setting the carriage in a position where its belt hooks don't engage belt
  // holes (because they are offset).
  km.putCarriageCenterInFrontOfNeedle(1);

  // So when it moves to the left, the belt doesn't move, its phase doesn't
  // change.
  while (km.moveCarriageCenterTowardsNeedle(0)) {
    ASSERT_FALSE(km.getBeltPhase());
  }

  // Now that the carriage is at a belt position divisible by 8, it's engaged
  // and the belt moves.
  while (km.moveCarriageCenterTowardsNeedle(-4)) {
  }
  ASSERT_TRUE(km.getBeltPhase());
}

/**
 * Helper to print the value of all signals while moving a carriage
 * across the bed.
 * The output can be extracted and compared with a similar scan
 * recorded on an actual machine.
 * Sample `ctest` invocation to extract data:
 *
 *   ctest --test-dir test/build -V -R KCarriageScanBed|grep SCAN=|cut -d= -f2
 */
void doBedScan(KnittingMachine &km) {
  float centerNeedle = -32;
  km.putCarriageCenterInFrontOfNeedle(centerNeedle);

  while (km.moveCarriageCenterTowardsNeedle(231)) {
    centerNeedle += 1.0 / 4;
    printf("SCAN=%g\t%g\t%g\t%g\t%d\n", centerNeedle,
           km.getLeftPositionSensorVoltage(),
           km.getRightPositionSensorVoltage(),
           km.getRightPositionSensorKSignal(), km.getBeltPhase() ? 5 : 0);
  }
}

TEST(KnittingMachine, GCarriageScanBed) {
  KnittingMachine km;

  km.addGCarriageMagnets();
  doBedScan(km);
}

TEST(KnittingMachine, KCarriageScanBed) {
  KnittingMachine km;

  km.addCarriageMagnet(0, true);
  doBedScan(km);
}