/*!
 * \file test_encoders.cpp
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

#include <gtest/gtest.h>

#include <board.h>
#include <encoders.h>

#include <analogReadAsyncWrapper_mock.h>

using ::testing::_;
using ::testing::AtLeast;
using ::testing::Mock;
using ::testing::Return;

extern Encoders *encoders;

extern AnalogReadAsyncWrapperMock *analogReadAsyncWrapper;

class EncodersTest : public ::testing::Test {
protected:
  void SetUp() override {
    arduinoMock = arduinoMockInstance();

    // pointers to global instances
    analogReadAsyncWrapperMock = analogReadAsyncWrapper;

    // The global instances do not get destroyed at the end of each test.
// Ordinarily the mock instances would be local and such behaviour would
    // cause a memory leak. We must notify the test that this is not the case.
    Mock::AllowLeak(analogReadAsyncWrapperMock);

    encoders->init(Machine_t::Kh910);
  }

  void TearDown() override {
    releaseArduinoMock();
  }

  ArduinoMock *arduinoMock;
  AnalogReadAsyncWrapperMock *analogReadAsyncWrapperMock;
};

TEST_F(EncodersTest, test_encA_rising_not_in_front) {
  // Create a falling edge
  EXPECT_CALL(*arduinoMock, digitalRead(ENC_PIN_A)).WillOnce(Return(false));
  // We should not enter the falling function
  EXPECT_CALL(*analogReadAsyncWrapperMock, analogReadAsyncWrapped).Times(0);
  encoders->isr();

  // Enter rising function
  EXPECT_CALL(*arduinoMock, digitalRead(ENC_PIN_A)).WillOnce(Return(true));
  EXPECT_CALL(*analogReadAsyncWrapperMock, analogReadAsyncWrapped);
  encoders->isr();
  // Direction is Right
  EXPECT_CALL(*arduinoMock, digitalRead(ENC_PIN_B)).WillOnce(Return(true));
  // BeltShift not decided
  EXPECT_CALL(*arduinoMock, digitalRead(ENC_PIN_C)).Times(0);
  // Not in front of Left Hall Sensor
  encoders->hallLeftCallback(FILTER_L_MIN[static_cast<int8_t>(encoders->getMachineType())], nullptr);

  ASSERT_EQ(encoders->getDirection(), Direction_t::Right);
  ASSERT_EQ(encoders->getPosition(), 0x01);
  ASSERT_EQ(encoders->getCarriage(), Carriage_t::NoCarriage);

  // Enter falling function
  EXPECT_CALL(*arduinoMock, digitalRead(ENC_PIN_A)).WillOnce(Return(false));
  EXPECT_CALL(*analogReadAsyncWrapperMock, analogReadAsyncWrapped);
  encoders->isr();
  // Direction is Right
  EXPECT_CALL(*arduinoMock, digitalRead(ENC_PIN_B)).WillOnce(Return(false));
  // Beltshift is decided
  EXPECT_CALL(*arduinoMock, digitalRead(ENC_PIN_C));
  // In front of Right Hall Sensor
  encoders->hallRightCallback(FILTER_R_MIN[static_cast<int8_t>(encoders->getMachineType())] - 1, nullptr);

  ASSERT_EQ(encoders->getDirection(), Direction_t::Right);
  ASSERT_EQ(encoders->getPosition(), END_RIGHT_MINUS_OFFSET[static_cast<int8_t>(encoders->getMachineType())]);
  ASSERT_EQ(encoders->getCarriage(), Carriage_t::Knit);

  // Enter rising function
  EXPECT_CALL(*arduinoMock, digitalRead(ENC_PIN_A)).WillOnce(Return(true));
  EXPECT_CALL(*analogReadAsyncWrapperMock, analogReadAsyncWrapped);
  encoders->isr();
  // Direction is Right
  EXPECT_CALL(*arduinoMock, digitalRead(ENC_PIN_B)).WillOnce(Return(true));
  // Beltshift is decided
  EXPECT_CALL(*arduinoMock, digitalRead(ENC_PIN_C));
  // In front of Left Hall Sensor
  encoders->hallLeftCallback(FILTER_L_MAX[static_cast<int8_t>(encoders->getMachineType())] + 1, nullptr);

  ASSERT_EQ(encoders->getDirection(), Direction_t::Right);
  ASSERT_EQ(encoders->getPosition(), END_LEFT_PLUS_OFFSET[static_cast<int8_t>(encoders->getMachineType())]);
  ASSERT_EQ(encoders->getCarriage(), Carriage_t::Knit);

  // Test expectations without destroying instance
  ASSERT_TRUE(Mock::VerifyAndClear(analogReadAsyncWrapperMock));
}

TEST_F(EncodersTest, test_encA_rising_in_front_notKH270) {
  ASSERT_FALSE(encoders->getMachineType() == Machine_t::Kh270);
  ASSERT_EQ(encoders->getCarriage(), Carriage_t::NoCarriage);

  // Create a falling edge
  EXPECT_CALL(*arduinoMock, digitalRead(ENC_PIN_A)).WillOnce(Return(false));
  // We should not enter the falling function
  EXPECT_CALL(*analogReadAsyncWrapperMock, analogReadAsyncWrapped).Times(0);
  encoders->isr();

  // Enter rising function
  EXPECT_CALL(*arduinoMock, digitalRead(ENC_PIN_A)).WillOnce(Return(true));
  EXPECT_CALL(*analogReadAsyncWrapperMock, analogReadAsyncWrapped);
  encoders->isr();
  // Direction is Right
  EXPECT_CALL(*arduinoMock, digitalRead(ENC_PIN_B)).WillOnce(Return(true));
  // BeltShift is regular
  EXPECT_CALL(*arduinoMock, digitalRead(ENC_PIN_C)).WillOnce(Return(true));
  // In front of Left Hall Sensor
  encoders->hallLeftCallback(FILTER_L_MIN[static_cast<int8_t>(encoders->getMachineType())] - 1, nullptr);

  ASSERT_EQ(encoders->getDirection(), Direction_t::Right);
  ASSERT_EQ(encoders->getHallActive(), Direction_t::Left);
  ASSERT_EQ(encoders->getPosition(), END_LEFT_PLUS_OFFSET[static_cast<int8_t>(encoders->getMachineType())]);
  ASSERT_EQ(encoders->getCarriage(), Carriage_t::Lace);
  ASSERT_EQ(encoders->getBeltShift(), BeltShift::Regular);

  // Enter falling function
  EXPECT_CALL(*arduinoMock, digitalRead(ENC_PIN_A)).WillOnce(Return(false));
  EXPECT_CALL(*analogReadAsyncWrapperMock, analogReadAsyncWrapped);
  encoders->isr();
  // Direction is Right
  EXPECT_CALL(*arduinoMock, digitalRead(ENC_PIN_B)).WillOnce(Return(false));
  // BeltShift is not decided
  EXPECT_CALL(*arduinoMock, digitalRead(ENC_PIN_C)).Times(0);
  // Not in front of Right Hall Sensor
  encoders->hallRightCallback(FILTER_R_MIN[static_cast<int8_t>(encoders->getMachineType())], nullptr);

  encoders->m_position = 0;
  // Enter rising function
  EXPECT_CALL(*arduinoMock, digitalRead(ENC_PIN_A)).WillOnce(Return(true));
  EXPECT_CALL(*analogReadAsyncWrapperMock, analogReadAsyncWrapped);
  encoders->isr();
  // Direction is Right
  EXPECT_CALL(*arduinoMock, digitalRead(ENC_PIN_B)).WillOnce(Return(true));
  // BeltShift is decided
  EXPECT_CALL(*arduinoMock, digitalRead(ENC_PIN_C));
  // In front of Left Hall Sensor
  encoders->hallLeftCallback(FILTER_L_MAX[static_cast<int8_t>(encoders->getMachineType())] + 1, nullptr);

  ASSERT_EQ(encoders->getDirection(), Direction_t::Right);
  ASSERT_EQ(encoders->getPosition(), END_LEFT_PLUS_OFFSET[static_cast<int8_t>(encoders->getMachineType())]);
  ASSERT_EQ(encoders->getCarriage(), Carriage_t::Knit);

  // Test expectations without destroying instance
  ASSERT_TRUE(Mock::VerifyAndClear(analogReadAsyncWrapperMock));
}

TEST_F(EncodersTest, test_encA_rising_in_front_KH270) {
  encoders->init(Machine_t::Kh270);
  ASSERT_EQ(encoders->getMachineType(), Machine_t::Kh270);

  // Create a rising edge
  EXPECT_CALL(*arduinoMock, digitalRead(ENC_PIN_A)).WillOnce(Return(false));
  // We should not enter the falling function
  EXPECT_CALL(*analogReadAsyncWrapperMock, analogReadAsyncWrapped).Times(0);
  encoders->isr();

  // Enter rising function
  EXPECT_CALL(*arduinoMock, digitalRead(ENC_PIN_A)).WillOnce(Return(true));
  EXPECT_CALL(*analogReadAsyncWrapperMock, analogReadAsyncWrapped);
  encoders->isr();
  // Direction is Right
  EXPECT_CALL(*arduinoMock, digitalRead(ENC_PIN_B)).WillOnce(Return(true));
  // BeltShift is ignored
  EXPECT_CALL(*arduinoMock, digitalRead(ENC_PIN_C)).Times(0);
  // In front of Left Hall Sensor
  encoders->hallLeftCallback(FILTER_L_MIN[static_cast<int8_t>(Machine_t::Kh270)] - 1, nullptr);

  ASSERT_EQ(encoders->getDirection(), Direction_t::Right);
  ASSERT_EQ(encoders->getHallActive(), Direction_t::Left);
  ASSERT_EQ(encoders->getPosition(), END_LEFT_PLUS_OFFSET[static_cast<int8_t>(Machine_t::Kh270)]);
  ASSERT_EQ(encoders->getCarriage(), Carriage_t::Knit);
  ASSERT_EQ(encoders->getBeltShift(), BeltShift::Unknown);

  // Enter falling function
  EXPECT_CALL(*arduinoMock, digitalRead(ENC_PIN_A)).WillOnce(Return(false));
  EXPECT_CALL(*analogReadAsyncWrapperMock, analogReadAsyncWrapped);
  encoders->isr();
  // Direction is Right
  EXPECT_CALL(*arduinoMock, digitalRead(ENC_PIN_B)).WillOnce(Return(false));
  // BeltShift is ignored
  EXPECT_CALL(*arduinoMock, digitalRead(ENC_PIN_C)).Times(0);
  // In front of Right Hall Sensor
  encoders->hallRightCallback(FILTER_R_MIN[static_cast<int8_t>(encoders->getMachineType())] - 1, nullptr);

  ASSERT_EQ(encoders->getDirection(), Direction_t::Right);
  ASSERT_EQ(encoders->getHallActive(), Direction_t::Right);
  ASSERT_EQ(encoders->getPosition(), END_RIGHT_MINUS_OFFSET[static_cast<int8_t>(Machine_t::Kh270)]);
  ASSERT_EQ(encoders->getCarriage(), Carriage_t::Knit);
  ASSERT_EQ(encoders->getBeltShift(), BeltShift::Unknown);

  // Create a rising edge
  EXPECT_CALL(*arduinoMock, digitalRead(ENC_PIN_A)).WillOnce(Return(true));
  // We will not enter the rising function
  EXPECT_CALL(*analogReadAsyncWrapperMock, analogReadAsyncWrapped).Times(0);
  encoders->isr();

  // Test expectations without destroying instance
  ASSERT_TRUE(Mock::VerifyAndClear(analogReadAsyncWrapperMock));
}

TEST_F(EncodersTest, test_encA_rising_after_KH270) {
  encoders->init(Machine_t::Kh270);
  ASSERT_EQ(encoders->getMachineType(), Machine_t::Kh270);

  // Create a falling edge
  EXPECT_CALL(*arduinoMock, digitalRead(ENC_PIN_A)).WillOnce(Return(false));
  // We should not enter the falling function
  EXPECT_CALL(*analogReadAsyncWrapperMock, analogReadAsyncWrapped).Times(0);
  encoders->isr();

  // Enter rising function
  EXPECT_CALL(*arduinoMock, digitalRead(ENC_PIN_A)).WillOnce(Return(true));
  EXPECT_CALL(*analogReadAsyncWrapperMock, analogReadAsyncWrapped);
  encoders->isr();
  // Direction is Right
  EXPECT_CALL(*arduinoMock, digitalRead(ENC_PIN_B)).WillOnce(Return(true));
  // BeltShift ignored
  EXPECT_CALL(*arduinoMock, digitalRead(ENC_PIN_C)).Times(0);
  // Not in front of Left Hall Sensor
  encoders->hallLeftCallback(FILTER_L_MAX[static_cast<int8_t>(encoders->getMachineType())] + 1, nullptr);

  ASSERT_EQ(encoders->getDirection(), Direction_t::Right);
  ASSERT_EQ(encoders->getHallActive(), Direction_t::Left);
  ASSERT_EQ(encoders->getPosition(), END_LEFT_PLUS_OFFSET[static_cast<int8_t>(Machine_t::Kh270)] + MAGNET_DISTANCE_270);
  ASSERT_EQ(encoders->getCarriage(), Carriage_t::Knit);
  ASSERT_EQ(encoders->getBeltShift(), BeltShift::Unknown);

  // Test expectations without destroying instance
  ASSERT_TRUE(Mock::VerifyAndClear(analogReadAsyncWrapperMock));
}

TEST_F(EncodersTest, test_G_carriage) {
  ASSERT_FALSE(encoders->getMachineType() == Machine_t::Kh270);
  ASSERT_EQ(encoders->getCarriage(), Carriage_t::NoCarriage);

  // Enter rising function
  EXPECT_CALL(*arduinoMock, digitalRead(ENC_PIN_A)).WillOnce(Return(true));
  EXPECT_CALL(*analogReadAsyncWrapperMock, analogReadAsyncWrapped);
  encoders->isr();
  // Direction is Right
  EXPECT_CALL(*arduinoMock, digitalRead(ENC_PIN_B)).WillOnce(Return(true));
  // BeltShift is regular
  EXPECT_CALL(*arduinoMock, digitalRead(ENC_PIN_C)).WillOnce(Return(true));
  // In front of Left Hall Sensor
  encoders->hallLeftCallback(FILTER_L_MAX[static_cast<int8_t>(encoders->getMachineType())] + 1, nullptr);

  ASSERT_EQ(encoders->getCarriage(), Carriage_t::Knit);

  // Enter falling function
  EXPECT_CALL(*arduinoMock, digitalRead(ENC_PIN_A)).WillOnce(Return(false));
  EXPECT_CALL(*analogReadAsyncWrapperMock, analogReadAsyncWrapped);
  encoders->isr();
  // Direction is Right
  EXPECT_CALL(*arduinoMock, digitalRead(ENC_PIN_B)).WillOnce(Return(false));
  // Beltshift is decided
  EXPECT_CALL(*arduinoMock, digitalRead(ENC_PIN_C));
  // In front of Right Hall Sensor
  encoders->hallRightCallback(FILTER_R_MAX[static_cast<int8_t>(encoders->getMachineType())] + 1, nullptr);

  // Enter rising function
  EXPECT_CALL(*arduinoMock, digitalRead(ENC_PIN_A)).WillOnce(Return(true));
  EXPECT_CALL(*analogReadAsyncWrapperMock, analogReadAsyncWrapped);
  encoders->isr();
  // Direction is Right
  EXPECT_CALL(*arduinoMock, digitalRead(ENC_PIN_B)).WillOnce(Return(true));
  // In front of Left Hall Sensor
  encoders->hallLeftCallback(FILTER_L_MIN[static_cast<int8_t>(encoders->getMachineType())] - 1, nullptr);

  ASSERT_EQ(encoders->getCarriage(), Carriage_t::Garter);

  // Enter falling function
  EXPECT_CALL(*arduinoMock, digitalRead(ENC_PIN_A)).WillOnce(Return(false));
  EXPECT_CALL(*analogReadAsyncWrapperMock, analogReadAsyncWrapped);
  encoders->isr();
  // Direction is Right
  EXPECT_CALL(*arduinoMock, digitalRead(ENC_PIN_B)).WillOnce(Return(false));
  // Beltshift is not decided
  EXPECT_CALL(*arduinoMock, digitalRead(ENC_PIN_C)).Times(0);
  // Not in front of Right Hall Sensor
  encoders->hallRightCallback(FILTER_R_MIN[static_cast<int8_t>(encoders->getMachineType())], nullptr);

  // Create a rising edge
  EXPECT_CALL(*arduinoMock, digitalRead(ENC_PIN_A)).WillOnce(Return(true));
  // We will not enter the rising function
  EXPECT_CALL(*analogReadAsyncWrapperMock, analogReadAsyncWrapped).Times(0);
  encoders->isr();

  // Create a falling edge
  EXPECT_CALL(*arduinoMock, digitalRead(ENC_PIN_A)).WillOnce(Return(false));
  EXPECT_CALL(*analogReadAsyncWrapperMock, analogReadAsyncWrapped);
  encoders->isr();
  // Direction is Right:
  EXPECT_CALL(*arduinoMock, digitalRead(ENC_PIN_B)).WillOnce(Return(false));
  // Beltshift is decided
  EXPECT_CALL(*arduinoMock, digitalRead(ENC_PIN_C));
  // In front of Right Hall Sensor
  encoders->hallRightCallback(FILTER_R_MIN[static_cast<int8_t>(encoders->getMachineType())] - 1, nullptr);

  // Test expectations without destroying instance
  ASSERT_TRUE(Mock::VerifyAndClear(analogReadAsyncWrapperMock));
}

TEST_F(EncodersTest, test_encA_falling_not_in_front) {
  // Rising edge
  EXPECT_CALL(*arduinoMock, digitalRead(ENC_PIN_A)).WillOnce(Return(true));
  EXPECT_CALL(*analogReadAsyncWrapperMock, analogReadAsyncWrapped);
  encoders->isr();
  // Direction is Right
  EXPECT_CALL(*arduinoMock, digitalRead(ENC_PIN_B)).WillOnce(Return(true));
  // Beltshift is not decided
  EXPECT_CALL(*arduinoMock, digitalRead(ENC_PIN_C)).Times(0);
  // Not in front of Left Hall Sensor
  encoders->hallLeftCallback(FILTER_L_MIN[static_cast<int8_t>(encoders->getMachineType())], nullptr);

  // Create rising edge
  EXPECT_CALL(*arduinoMock, digitalRead(ENC_PIN_A)).WillOnce(Return(true));
  // Rising function not entered
  EXPECT_CALL(*analogReadAsyncWrapperMock, analogReadAsyncWrapped).Times(0);
  encoders->isr();

  // Falling edge
  encoders->m_position = END_LEFT[static_cast<uint8_t>(encoders->getMachineType())] + 1;
  EXPECT_CALL(*arduinoMock, digitalRead(ENC_PIN_A)).WillOnce(Return(false));
  EXPECT_CALL(*analogReadAsyncWrapperMock, analogReadAsyncWrapped);
  encoders->isr();
  // Direction is Left
  EXPECT_CALL(*arduinoMock, digitalRead(ENC_PIN_B)).WillOnce(Return(true));
  // Beltshift is not decided
  EXPECT_CALL(*arduinoMock, digitalRead(ENC_PIN_C)).Times(0);
  // Not in front of Right Hall Sensor
  encoders->hallRightCallback(FILTER_R_MIN[static_cast<int8_t>(encoders->getMachineType())], nullptr);

  // Test expectations without destroying instance
  ASSERT_TRUE(Mock::VerifyAndClear(analogReadAsyncWrapperMock));
}

TEST_F(EncodersTest, test_encA_falling_in_front) {
  ASSERT_FALSE(encoders->getMachineType() == Machine_t::Kh270);
  ASSERT_EQ(encoders->getCarriage(), Carriage_t::NoCarriage);

  // Enter rising function
  EXPECT_CALL(*arduinoMock, digitalRead(ENC_PIN_A)).WillOnce(Return(true));
  EXPECT_CALL(*analogReadAsyncWrapperMock, analogReadAsyncWrapped);
  encoders->isr();
  // Direction is Left
  EXPECT_CALL(*arduinoMock, digitalRead(ENC_PIN_B)).WillOnce(Return(false));
  // Beltshift is not decided
  EXPECT_CALL(*arduinoMock, digitalRead(ENC_PIN_C)).Times(0);
  // Not in front of Left Hall Sensor
  encoders->hallLeftCallback(FILTER_L_MIN[static_cast<int8_t>(encoders->getMachineType())], nullptr);

  // Falling edge
  EXPECT_CALL(*arduinoMock, digitalRead(ENC_PIN_A)).WillOnce(Return(false));
  EXPECT_CALL(*analogReadAsyncWrapperMock, analogReadAsyncWrapped);
  encoders->isr();
  // Direction is Left
  EXPECT_CALL(*arduinoMock, digitalRead(ENC_PIN_B)).WillOnce(Return(false));
  // BeltShift is shifted
  EXPECT_CALL(*arduinoMock, digitalRead(ENC_PIN_C)).WillOnce(Return(true));
  // In front of Right Hall Sensor
  encoders->hallRightCallback(FILTER_R_MAX[static_cast<int8_t>(encoders->getMachineType())] + 1, nullptr);

  ASSERT_EQ(encoders->getDirection(), Direction_t::Right);
  ASSERT_EQ(encoders->getHallActive(), Direction_t::Right);
  ASSERT_EQ(encoders->getPosition(), END_RIGHT_MINUS_OFFSET[static_cast<int8_t>(encoders->getMachineType())]);
  ASSERT_EQ(encoders->getCarriage(), Carriage_t::NoCarriage);
  ASSERT_EQ(encoders->getBeltShift(), BeltShift::Shifted);

  // Test expectations without destroying instance
  ASSERT_TRUE(Mock::VerifyAndClear(analogReadAsyncWrapperMock));
}

TEST_F(EncodersTest, test_encA_falling_at_end) {
  ASSERT_FALSE(encoders->getMachineType() == Machine_t::Kh270);
  ASSERT_EQ(encoders->getCarriage(), Carriage_t::NoCarriage);

  // Rising edge
  EXPECT_CALL(*arduinoMock, digitalRead(ENC_PIN_A)).WillOnce(Return(true));
  EXPECT_CALL(*analogReadAsyncWrapperMock, analogReadAsyncWrapped);
  encoders->isr();
  // Direction is Left
  EXPECT_CALL(*arduinoMock, digitalRead(ENC_PIN_B)).WillOnce(Return(false));
  // Beltshift is not decided
  EXPECT_CALL(*arduinoMock, digitalRead(ENC_PIN_C)).Times(0);
  // Not in front of Left Hall Sensor
  encoders->hallLeftCallback(FILTER_L_MAX[static_cast<int8_t>(encoders->getMachineType())], nullptr);

  // Falling edge
  EXPECT_CALL(*arduinoMock, digitalRead(ENC_PIN_A)).WillOnce(Return(false));
  EXPECT_CALL(*analogReadAsyncWrapperMock, analogReadAsyncWrapped);
  encoders->isr();
  // Direction is Left
  EXPECT_CALL(*arduinoMock, digitalRead(ENC_PIN_B)).WillOnce(Return(true));
  // Beltshift is decided
  EXPECT_CALL(*arduinoMock, digitalRead(ENC_PIN_C));
  // In front of Right Hall Sensor
  encoders->hallRightCallback(FILTER_R_MAX[static_cast<int8_t>(encoders->getMachineType())] + 1, nullptr);

  ASSERT_EQ(encoders->getPosition(), END_RIGHT_MINUS_OFFSET[static_cast<int8_t>(encoders->getMachineType())]);

  uint16_t pos = END_RIGHT_MINUS_OFFSET[static_cast<int8_t>(encoders->getMachineType())];
  while (pos < END_RIGHT[static_cast<int8_t>(encoders->getMachineType())]) {
    // Rising edge
    EXPECT_CALL(*arduinoMock, digitalRead(ENC_PIN_A)).WillOnce(Return(true));
    EXPECT_CALL(*analogReadAsyncWrapperMock, analogReadAsyncWrapped);
    encoders->isr();
    // Direction is Right
    EXPECT_CALL(*arduinoMock, digitalRead(ENC_PIN_B)).WillOnce(Return(true));
    // Beltshift is not decided
    EXPECT_CALL(*arduinoMock, digitalRead(ENC_PIN_C)).Times(0);
    // Not in front of Left Hall Sensor
    encoders->hallLeftCallback(FILTER_L_MAX[static_cast<int8_t>(encoders->getMachineType())], nullptr);

    ASSERT_EQ(encoders->getPosition(), ++pos);

    // Falling edge
    EXPECT_CALL(*arduinoMock, digitalRead(ENC_PIN_A)).WillOnce(Return(false));
    EXPECT_CALL(*analogReadAsyncWrapperMock, analogReadAsyncWrapped);
    encoders->isr();
    // Direction does not matter
    EXPECT_CALL(*arduinoMock, digitalRead(ENC_PIN_B));
    // Beltshift is not decided
    EXPECT_CALL(*arduinoMock, digitalRead(ENC_PIN_C)).Times(0);
    // Not in front of Right Hall Sensor
    encoders->hallRightCallback(FILTER_R_MAX[static_cast<int8_t>(encoders->getMachineType())], nullptr);

    ASSERT_EQ(encoders->getPosition(), pos);
  }

  ASSERT_EQ(encoders->getPosition(), pos);

  // Rising edge
  EXPECT_CALL(*arduinoMock, digitalRead(ENC_PIN_A)).WillOnce(Return(true));
  EXPECT_CALL(*analogReadAsyncWrapperMock, analogReadAsyncWrapped);
  encoders->isr();
  // Direction is Right
  EXPECT_CALL(*arduinoMock, digitalRead(ENC_PIN_B)).WillOnce(Return(true));
  // Beltshift is not decided
  EXPECT_CALL(*arduinoMock, digitalRead(ENC_PIN_C)).Times(0);
  // Not in front of Left Hall Sensor
  encoders->hallLeftCallback(FILTER_L_MAX[static_cast<int8_t>(encoders->getMachineType())], nullptr);

  ASSERT_EQ(encoders->getPosition(), pos);

  // Test expectations without destroying instance
  ASSERT_TRUE(Mock::VerifyAndClear(analogReadAsyncWrapperMock));
}

// requires FILTER_R_MIN != 0
TEST_F(EncodersTest, test_encA_falling_set_K_carriage_KH910) {
  ASSERT_EQ(encoders->getMachineType(), Machine_t::Kh910);

  // Rising edge
  EXPECT_CALL(*arduinoMock, digitalRead(ENC_PIN_A)).WillOnce(Return(true));
  EXPECT_CALL(*analogReadAsyncWrapperMock, analogReadAsyncWrapped);
  encoders->isr();
  // Direction does not matter
  EXPECT_CALL(*arduinoMock, digitalRead(ENC_PIN_B));
  // Beltshift is not decided
  EXPECT_CALL(*arduinoMock, digitalRead(ENC_PIN_C)).Times(0);
  // Not in front of Left Hall Sensor
  encoders->hallLeftCallback(FILTER_L_MIN[static_cast<int8_t>(encoders->getMachineType())], nullptr);

  // Falling edge
  EXPECT_CALL(*arduinoMock, digitalRead(ENC_PIN_A)).WillOnce(Return(false));
  EXPECT_CALL(*analogReadAsyncWrapperMock, analogReadAsyncWrapped);
  encoders->isr();
  // Direction does not matter
  EXPECT_CALL(*arduinoMock, digitalRead(ENC_PIN_B));
  // Beltshift is decided
  EXPECT_CALL(*arduinoMock, digitalRead(ENC_PIN_C));
  // In front of Right Hall Sensor
  encoders->hallRightCallback(FILTER_R_MIN[static_cast<int8_t>(encoders->getMachineType())] - 1, nullptr);

  ASSERT_EQ(encoders->getCarriage(), Carriage_t::Knit);

  // Test expectations without destroying instance
  ASSERT_TRUE(Mock::VerifyAndClear(analogReadAsyncWrapperMock));
}

TEST_F(EncodersTest, test_encA_falling_not_at_end) {
  ASSERT_FALSE(encoders->getMachineType() == Machine_t::Kh270);
  ASSERT_EQ(encoders->getCarriage(), Carriage_t::NoCarriage);

  // Rising edge
  EXPECT_CALL(*arduinoMock, digitalRead(ENC_PIN_A)).WillOnce(Return(true));
  EXPECT_CALL(*analogReadAsyncWrapperMock, analogReadAsyncWrapped);
  encoders->isr();
  // Direction is Left
  EXPECT_CALL(*arduinoMock, digitalRead(ENC_PIN_B)).WillOnce(Return(false));
  // Beltshift is decided
  EXPECT_CALL(*arduinoMock, digitalRead(ENC_PIN_C));
  // In front of Left Hall Sensor
  encoders->hallLeftCallback(FILTER_R_MAX[static_cast<int8_t>(encoders->getMachineType())] + 1, nullptr);

  ASSERT_EQ(encoders->getPosition(), END_LEFT_PLUS_OFFSET[static_cast<int8_t>(encoders->getMachineType())]);

  // Falling edge
  EXPECT_CALL(*arduinoMock, digitalRead(ENC_PIN_A)).WillOnce(Return(false));
  EXPECT_CALL(*analogReadAsyncWrapperMock, analogReadAsyncWrapped);
  encoders->isr();
  // Direction is Right
  EXPECT_CALL(*arduinoMock, digitalRead(ENC_PIN_B)).WillOnce(Return(false));
  // Beltshift is not decided
  EXPECT_CALL(*arduinoMock, digitalRead(ENC_PIN_C)).Times(0);
  // Not in front of Right Hall Sensor
  encoders->hallRightCallback(FILTER_R_MAX[static_cast<int8_t>(encoders->getMachineType())], nullptr);

  ASSERT_EQ(encoders->getPosition(), END_LEFT_PLUS_OFFSET[static_cast<int8_t>(encoders->getMachineType())]);

  // Test expectations without destroying instance
  ASSERT_TRUE(Mock::VerifyAndClear(analogReadAsyncWrapperMock));
}

TEST_F(EncodersTest, test_getPosition) {
  uint8_t p = encoders->getPosition();
  ASSERT_EQ(p, 0x00);
}

TEST_F(EncodersTest, test_getBeltShift) {
  BeltShift_t b = encoders->getBeltShift();
  ASSERT_EQ(b, BeltShift::Unknown);
}

TEST_F(EncodersTest, test_getDirection) {
  Direction_t d = encoders->getDirection();
  ASSERT_EQ(d, Direction_t::NoDirection);
}

TEST_F(EncodersTest, test_getHallActive) {
  Direction_t d = encoders->getHallActive();
  ASSERT_EQ(d, Direction_t::NoDirection);
}

TEST_F(EncodersTest, test_getCarriage) {
  Carriage_t c = encoders->getCarriage();
  ASSERT_EQ(c, Carriage_t::NoCarriage);
}

TEST_F(EncodersTest, test_getMachineType) {
  Machine_t m = encoders->getMachineType();
  ASSERT_EQ(m, Machine_t::Kh910);
}

TEST_F(EncodersTest, test_init) {
  encoders->init(Machine_t::Kh270);
  Machine_t m = encoders->getMachineType();
  ASSERT_EQ(m, Machine_t::Kh270);
}

TEST_F(EncodersTest, test_getHallValue) {
  uint16_t v = encoders->getHallValue(Direction_t::NoDirection);
  ASSERT_EQ(v, 0U);

  EXPECT_CALL(*arduinoMock, analogRead(EOL_PIN_L));
  v = encoders->getHallValue(Direction_t::Left);
  ASSERT_EQ(v, 0U);

  EXPECT_CALL(*arduinoMock, analogRead(EOL_PIN_R));
  v = encoders->getHallValue(Direction_t::Right);
  ASSERT_EQ(v, 0U);

  EXPECT_CALL(*arduinoMock, analogRead(EOL_PIN_R)).WillOnce(Return(0xBEEFU));
  v = encoders->getHallValue(Direction_t::Right);
  ASSERT_EQ(v, 0xBEEFU);
}
