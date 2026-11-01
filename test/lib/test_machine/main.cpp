#include <gtest/gtest.h>

#include "machine.h"

TEST(MachineTest, StartsUndefined) {
  Machine machine;

  EXPECT_FALSE(machine.isDefined());
  EXPECT_EQ(machine.getType(), MachineType::NoMachine);
}

TEST(MachineTest, ReportsNeedleCountForEachMachine) {
  Machine machine;

  machine.setType(MachineType::Kh910);
  EXPECT_EQ(machine.getNumberofNeedles(), 200);

  machine.setType(MachineType::Kh930);
  EXPECT_EQ(machine.getNumberofNeedles(), 200);

  machine.setType(MachineType::Kh270);
  EXPECT_EQ(machine.getNumberofNeedles(), 112);
}

TEST(MachineTest, ConfiguresKh910RightSensorAsDigital) {
  Machine machine;
  machine.setType(MachineType::Kh910);

  HallSensor::Config *left = machine.getSensorConfig(MachineSide::Left);
  HallSensor::Config *right = machine.getSensorConfig(MachineSide::Right);

  EXPECT_EQ(left->position, 0);
  EXPECT_EQ(left->flags, 0);
  EXPECT_EQ(right->position, 199);
  EXPECT_EQ(right->flags, HALLSENSOR_DIGITAL);
  EXPECT_EQ(right->thresholdLow, 200);
  EXPECT_EQ(right->thresholdHigh, 600);
}

TEST(MachineTest, ConfiguresKh930SensorsAsAnalog) {
  Machine machine;
  machine.setType(MachineType::Kh930);

  HallSensor::Config *left = machine.getSensorConfig(MachineSide::Left);
  HallSensor::Config *right = machine.getSensorConfig(MachineSide::Right);

  EXPECT_EQ(left->flags, 0);
  EXPECT_EQ(right->flags, 0);
}

TEST(MachineTest, ConfiguresKh270SensorPositionsAndDetection) {
  Machine machine;
  machine.setType(MachineType::Kh270);

  HallSensor::Config *left = machine.getSensorConfig(MachineSide::Left);
  HallSensor::Config *right = machine.getSensorConfig(MachineSide::Right);

  EXPECT_EQ(left->position, 0);
  EXPECT_EQ(right->position, 111);
  EXPECT_EQ(left->flags, HALLSENSOR_K270_K);
  EXPECT_EQ(right->flags, HALLSENSOR_K270_K);
}

TEST(MachineTest, MapsStandardMachineSolenoids) {
  Machine machine;
  machine.setType(MachineType::Kh930);

  EXPECT_EQ(machine.solenoidToSet(-1), 15);
  EXPECT_EQ(machine.solenoidToSet(0), 0);
  EXPECT_EQ(machine.solenoidToSet(15), 15);
  EXPECT_EQ(machine.solenoidToSet(16), 0);

  uint8_t solenoid = 3;
  machine.solenoidShift(solenoid);
  EXPECT_EQ(solenoid, 11);
  machine.solenoidMap(solenoid);
  EXPECT_EQ(solenoid, 11);
}

TEST(MachineTest, MapsKh270NeedlesToTwelvePhysicalSolenoids) {
  Machine machine;
  machine.setType(MachineType::Kh270);

  EXPECT_EQ(machine.solenoidToSet(-5), 11);
  EXPECT_EQ(machine.solenoidToSet(-4), 0);
  EXPECT_EQ(machine.solenoidToSet(0), 4);
  EXPECT_EQ(machine.solenoidToSet(7), 11);
  EXPECT_EQ(machine.solenoidToSet(8), 0);

  uint8_t solenoid = 2;
  machine.solenoidShift(solenoid);
  EXPECT_EQ(solenoid, 8);
  machine.solenoidMap(solenoid);
  EXPECT_EQ(solenoid, 11);
}
