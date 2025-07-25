/*!`s
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
 *    Modified Work Copyright 2020 Sturla Lange, Tom Price
 *    http://ayab-knitting.com
 */

#include <gtest/gtest.h>

#include <board.h>
#include <encoders.h>

using namespace ::testing;

extern Encoders *encoders;

const int MID_SENSOR_VALUE = 400;

class EncodersTest : public ::testing::Test {
protected:
  void SetUp() override {
    arduinoMock = arduinoMockInstance();

    encoders->init(Machine_t::Kh930);
  }

  void TearDown() override {
    releaseArduinoMock();
  }

  ArduinoMock *arduinoMock;
};

TEST_F(EncodersTest, test_encA_rising_not_in_front) {
  // We should not enter the falling function
  EXPECT_CALL(*arduinoMock, analogRead(EOL_PIN_R)).Times(0);
  // Create a rising edge
  EXPECT_CALL(*arduinoMock, digitalRead(ENC_PIN_A))
      .WillOnce(Return(false))
      .WillOnce(Return(true));
  // We have not entered the rising function yet
  EXPECT_CALL(*arduinoMock, analogRead(EOL_PIN_L)).Times(0);
  encoders->encA_interrupt();
  // Enter rising function, direction is right
  EXPECT_CALL(*arduinoMock, digitalRead(ENC_PIN_B)).WillOnce(Return(true));
  // Not in front of Left Hall Sensor
  EXPECT_CALL(*arduinoMock, analogRead(EOL_PIN_L))
      .WillOnce(Return(FILTER_L_MIN[static_cast<int8_t>(encoders->getMachineType())]));
  encoders->encA_interrupt();
  ASSERT_EQ(encoders->getDirection(), Direction_t::Right);
  ASSERT_EQ(encoders->getPosition(), 0x01);
  ASSERT_EQ(encoders->getCarriage(), Carriage_t::NoCarriage);
}

TEST_F(EncodersTest, test_encA_rising_in_front_notKH270) {
  encoders->init(Machine_t::Kh930);
  ASSERT_EQ(encoders->getCarriage(), Carriage_t::NoCarriage);
  // Not in front of Right Hall Sensor
  EXPECT_CALL(*arduinoMock, analogRead(EOL_PIN_R))
      .WillRepeatedly(Return(MID_SENSOR_VALUE));
  // In front of Left Hall Sensor
  EXPECT_CALL(*arduinoMock, analogRead(EOL_PIN_L))
      .WillRepeatedly(Return(FILTER_L_MIN[static_cast<int8_t>(encoders->getMachineType())] - 1));
  // BeltShift is shifted
  EXPECT_CALL(*arduinoMock, digitalRead(ENC_PIN_C)).WillRepeatedly(Return(HIGH));

  // Create two rising edges (the initial state of A is assumed to be low)
  EXPECT_CALL(*arduinoMock, digitalRead(ENC_PIN_A))
    .WillRepeatedly(ReturnRoundRobin({ HIGH, LOW }));
    
  // Always moving to the right: A == B
  EXPECT_CALL(*arduinoMock, digitalRead(ENC_PIN_B))
    .WillRepeatedly(ReturnRoundRobin({ HIGH, LOW }));

  // Process rising edge
  encoders->encA_interrupt();

  uint8_t startPosition = END_LEFT_PLUS_OFFSET[static_cast<int8_t>(encoders->getMachineType())];

  ASSERT_EQ(encoders->getDirection(), Direction_t::Right);
  ASSERT_EQ(encoders->getHallActive(), Direction_t::Left);
  ASSERT_EQ(encoders->getCarriage(), Carriage_t::Lace);
  ASSERT_EQ(encoders->getBeltShift(), BeltShift::Shifted);
  ASSERT_EQ(encoders->getPosition(), startPosition);

  // Process falling edge
  encoders->encA_interrupt();

  // Process rising edge
  encoders->encA_interrupt();

  // Should have moved and not reset position
  ASSERT_EQ(encoders->getPosition(), 1 + startPosition);
}

TEST_F(EncodersTest, test_encA_rising_in_front_KH270) {
  encoders->init(Machine_t::Kh270);
  ASSERT_EQ(encoders->getCarriage(), Carriage_t::NoCarriage);
  // Not in front of Right Hall Sensor
  EXPECT_CALL(*arduinoMock, analogRead(EOL_PIN_R))
      .WillRepeatedly(Return(MID_SENSOR_VALUE));
  // In front of Left Hall Sensor
  EXPECT_CALL(*arduinoMock, analogRead(EOL_PIN_L))
      .WillRepeatedly(Return(FILTER_L_MIN[static_cast<int8_t>(encoders->getMachineType())] - 1));
  // KH270 has no belt shift
  EXPECT_CALL(*arduinoMock, digitalRead(ENC_PIN_C)).WillRepeatedly(Return(HIGH));

  // Create two rising edges (the initial state of A is assumed to be low)
  EXPECT_CALL(*arduinoMock, digitalRead(ENC_PIN_A))
    .WillRepeatedly(ReturnRoundRobin({ HIGH, LOW }));

  // Always moving to the right: A == B
  EXPECT_CALL(*arduinoMock, digitalRead(ENC_PIN_B))
    .WillRepeatedly(ReturnRoundRobin({ HIGH, LOW }));

  // Process rising edge
  encoders->encA_interrupt();

  uint8_t startPosition = END_LEFT_PLUS_OFFSET[static_cast<int8_t>(encoders->getMachineType())];

  ASSERT_EQ(encoders->getDirection(), Direction_t::Right);
  ASSERT_EQ(encoders->getHallActive(), Direction_t::Left);
  ASSERT_EQ(encoders->getCarriage(), Carriage_t::Lace);
  ASSERT_EQ(encoders->getPosition(), startPosition);

  // Process falling edge
  encoders->encA_interrupt();

  // Process rising edge
  encoders->encA_interrupt();

  // Should have moved and not reset position
  ASSERT_EQ(encoders->getPosition(), 1 + startPosition);
}

TEST_F(EncodersTest, test_encA_rising_in_front_G_carriage) {
  // Create a rising edge
  EXPECT_CALL(*arduinoMock, digitalRead(ENC_PIN_A)).WillOnce(Return(true));
  // Enter rising function, direction is right
  EXPECT_CALL(*arduinoMock, digitalRead(ENC_PIN_B)).WillOnce(Return(true));
  // In front of Left Hall Sensor
  EXPECT_CALL(*arduinoMock, analogRead(EOL_PIN_L))
      .WillOnce(Return(FILTER_L_MIN[static_cast<int8_t>(encoders->getMachineType())] - 1));
  // BeltShift is regular
  EXPECT_CALL(*arduinoMock, digitalRead(ENC_PIN_C)).WillOnce(Return(true));

  encoders->encA_interrupt();

  ASSERT_EQ(encoders->getCarriage(), Carriage_t::Lace);

  // Create a falling edge
  EXPECT_CALL(*arduinoMock, digitalRead(ENC_PIN_A)).WillOnce(Return(false));
  // Enter falling function, direction is right
  EXPECT_CALL(*arduinoMock, digitalRead(ENC_PIN_B)).WillOnce(Return(false));
  // Not in front of Right Hall sensor
  EXPECT_CALL(*arduinoMock, analogRead(EOL_PIN_R))
      .WillOnce(Return(FILTER_R_MAX[static_cast<int8_t>(encoders->getMachineType())] - 1));

  encoders->encA_interrupt();

  // Create a rising edge
  EXPECT_CALL(*arduinoMock, digitalRead(ENC_PIN_A)).WillOnce(Return(true));
  // Enter rising function, direction is right
  EXPECT_CALL(*arduinoMock, digitalRead(ENC_PIN_B)).WillOnce(Return(true));
  // In front of Left Hall Sensor
  EXPECT_CALL(*arduinoMock, analogRead(EOL_PIN_L))
      .WillOnce(Return(FILTER_R_MAX[static_cast<int8_t>(encoders->getMachineType())] + 1));

  encoders->encA_interrupt();

  ASSERT_EQ(encoders->getCarriage(), Carriage_t::Garter);
}

TEST_F(EncodersTest, test_encA_falling_not_in_front) {
  // Create a falling edge
  EXPECT_CALL(*arduinoMock, digitalRead(ENC_PIN_A))
      .WillOnce(Return(true))
      .WillOnce(Return(true));
  // We have not entered the falling function yet
  EXPECT_CALL(*arduinoMock, analogRead(EOL_PIN_R)).Times(0);

  // Enter rising function, direction is right
  EXPECT_CALL(*arduinoMock, digitalRead(ENC_PIN_B)).WillOnce(Return(true)).WillOnce(Return(false));
  // Not in front of Left Hall Sensor
  EXPECT_CALL(*arduinoMock, analogRead(EOL_PIN_L))
      .WillOnce(Return(FILTER_L_MIN[static_cast<int8_t>(encoders->getMachineType())]));
  encoders->encA_interrupt();
  encoders->encA_interrupt();

  EXPECT_CALL(*arduinoMock, digitalRead(ENC_PIN_A)).WillOnce(Return(false));
  EXPECT_CALL(*arduinoMock, analogRead(EOL_PIN_R))
      .WillOnce(Return(FILTER_R_MIN[static_cast<int8_t>(encoders->getMachineType())]));

  encoders->encA_interrupt();
}

TEST_F(EncodersTest, test_encA_falling_in_front) {
  encoders->init(Machine_t::Kh930);
  ASSERT_EQ(encoders->getCarriage(), Carriage_t::NoCarriage);
  // Not in front of Left Hall Sensor
  EXPECT_CALL(*arduinoMock, analogRead(EOL_PIN_L))
      .WillRepeatedly(Return(MID_SENSOR_VALUE));
  // In front of Right Hall Sensor
  EXPECT_CALL(*arduinoMock, analogRead(EOL_PIN_R))
      .WillRepeatedly(Return(FILTER_R_MIN[static_cast<int8_t>(encoders->getMachineType())] - 1));
  // BeltShift is shifted
  EXPECT_CALL(*arduinoMock, digitalRead(ENC_PIN_C)).WillRepeatedly(Return(LOW));

  // Create two falling edges (the initial state of A is assumed to be low)
  EXPECT_CALL(*arduinoMock, digitalRead(ENC_PIN_A))
    .WillRepeatedly(ReturnRoundRobin({ HIGH, LOW }));
    
  // Always moving to the left: A != B
  EXPECT_CALL(*arduinoMock, digitalRead(ENC_PIN_B))
    .WillRepeatedly(ReturnRoundRobin({ LOW, HIGH }));

  // Process rising edge
  encoders->encA_interrupt();

  // Process falling edge
  encoders->encA_interrupt();

  uint8_t startPosition = END_RIGHT_MINUS_OFFSET[static_cast<int8_t>(encoders->getMachineType())];

  ASSERT_EQ(encoders->getDirection(), Direction_t::Left);
  ASSERT_EQ(encoders->getHallActive(), Direction_t::Right);
  ASSERT_EQ(encoders->getCarriage(), Carriage_t::Lace);
  ASSERT_EQ(encoders->getBeltShift(), BeltShift::Shifted);
  ASSERT_EQ(encoders->getPosition(), startPosition);

  // Process rising edge
  encoders->encA_interrupt();

  // Process falling edge
  encoders->encA_interrupt();

  // Should have moved and not reset position
  ASSERT_EQ(encoders->getPosition(), startPosition - 1);
}

TEST_F(EncodersTest, test_encA_falling_in_front_KH910_unfixed_K) {
  encoders->init(Machine_t::Kh910);
  ASSERT_EQ(encoders->getCarriage(), Carriage_t::NoCarriage);

  // Unfixed shield: lace signal is always high
  EXPECT_CALL(*arduinoMock, digitalRead(EOL_PIN_R_L))
    .WillRepeatedly(Return(HIGH));

  // Not in front of Left Hall Sensor
  EXPECT_CALL(*arduinoMock, analogRead(EOL_PIN_L))
      .WillRepeatedly(Return(MID_SENSOR_VALUE));

  // K carriage in front of Right Hall Sensor: on a 910, the right sensor only
  // triggers for the K carriage and with a low voltage
  EXPECT_CALL(*arduinoMock, digitalRead(EOL_PIN_R))
      .WillRepeatedly(Return(LOW));

  // BeltShift is shifted
  EXPECT_CALL(*arduinoMock, digitalRead(ENC_PIN_C)).WillRepeatedly(Return(LOW));

  // Create two falling edges (the initial state of A is assumed to be low)
  EXPECT_CALL(*arduinoMock, digitalRead(ENC_PIN_A))
    .WillRepeatedly(ReturnRoundRobin({ HIGH, LOW }));
    
  // Always moving to the left: A != B
  EXPECT_CALL(*arduinoMock, digitalRead(ENC_PIN_B))
    .WillRepeatedly(ReturnRoundRobin({ LOW, HIGH }));

  // Process rising edge
  encoders->encA_interrupt();

  // Process falling edge
  encoders->encA_interrupt();

  uint8_t startPosition = END_RIGHT_MINUS_OFFSET[static_cast<int8_t>(encoders->getMachineType())];

  ASSERT_EQ(encoders->getDirection(), Direction_t::Left);
  ASSERT_EQ(encoders->getHallActive(), Direction_t::Right);
  ASSERT_EQ(encoders->getCarriage(), Carriage_t::Knit);
  ASSERT_EQ(encoders->getBeltShift(), BeltShift::Shifted);
  ASSERT_EQ(encoders->getPosition(), startPosition);

  // Process rising edge
  encoders->encA_interrupt();

  // Process falling edge
  encoders->encA_interrupt();

  // Should have moved and not reset position
  ASSERT_EQ(encoders->getPosition(), startPosition - 1);
}

TEST_F(EncodersTest, test_encA_falling_in_front_KH910_fixed_L) {
  encoders->init(Machine_t::Kh910);
  ASSERT_EQ(encoders->getCarriage(), Carriage_t::NoCarriage);

  uint8_t detectPinLevel = HIGH;
  uint8_t detectPinMode = INPUT;

  // Record writes to fix detection pin (8) to reflect them on pin 7 (lace signal)
  EXPECT_CALL(*arduinoMock, pinMode(EOL_PIN_R_DETECT, _))
    .WillRepeatedly(SaveArg<1>(&detectPinMode));
  EXPECT_CALL(*arduinoMock, digitalWrite(EOL_PIN_R_DETECT, _))
    .WillRepeatedly(SaveArg<1>(&detectPinLevel));

  EXPECT_CALL(*arduinoMock, pinMode(EOL_PIN_R_L, INPUT_PULLUP))
    .Times(AtLeast(1));

  // Fixed shield: lace signal (pin 7) is low if pin 8 is pulled down,
  // and high if pin 8 is left floating which happens when the L carriage
  // is in front of the right Hall sensor
  EXPECT_CALL(*arduinoMock, digitalRead(EOL_PIN_R_L))
    .WillRepeatedly([&] {
      return (detectPinMode == OUTPUT && detectPinLevel == LOW) ? LOW : HIGH;
    });

  // Not in front of Left Hall Sensor
  EXPECT_CALL(*arduinoMock, analogRead(EOL_PIN_L))
      .WillRepeatedly(Return(MID_SENSOR_VALUE));

  // BeltShift is shifted
  EXPECT_CALL(*arduinoMock, digitalRead(ENC_PIN_C)).WillRepeatedly(Return(LOW));

  // Create two falling edges (the initial state of A is assumed to be low)
  EXPECT_CALL(*arduinoMock, digitalRead(ENC_PIN_A))
    .WillRepeatedly(ReturnRoundRobin({ HIGH, LOW }));
    
  // Always moving to the left: A != B
  EXPECT_CALL(*arduinoMock, digitalRead(ENC_PIN_B))
    .WillRepeatedly(ReturnRoundRobin({ LOW, HIGH }));

  // Process rising edge
  encoders->encA_interrupt();

  // Process falling edge
  encoders->encA_interrupt();

  uint8_t startPosition = END_RIGHT_MINUS_OFFSET[static_cast<int8_t>(encoders->getMachineType())];

  ASSERT_EQ(encoders->getDirection(), Direction_t::Left);
  ASSERT_EQ(encoders->getHallActive(), Direction_t::Right);
  ASSERT_EQ(encoders->getCarriage(), Carriage_t::Lace);
  ASSERT_EQ(encoders->getBeltShift(), BeltShift::Shifted);
  ASSERT_EQ(encoders->getPosition(), startPosition);

  // Process rising edge
  encoders->encA_interrupt();

  // Process falling edge
  encoders->encA_interrupt();

  // Should have moved and not reset position
  ASSERT_EQ(encoders->getPosition(), startPosition - 1);
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
  ASSERT_EQ(m, Machine_t::Kh930);
}

TEST_F(EncodersTest, test_getHallValue) {
  uint16_t v = encoders->getHallValue(Direction_t::NoDirection);
  ASSERT_EQ(v, 0u);
  EXPECT_CALL(*arduinoMock, analogRead(EOL_PIN_L));
  v = encoders->getHallValue(Direction_t::Left);
  ASSERT_EQ(v, 0u);
  EXPECT_CALL(*arduinoMock, analogRead(EOL_PIN_R));
  v = encoders->getHallValue(Direction_t::Right);
  ASSERT_EQ(v, 0u);
  EXPECT_CALL(*arduinoMock, analogRead(EOL_PIN_R)).WillOnce(Return(0xbeefu));
  v = encoders->getHallValue(Direction_t::Right);
  ASSERT_EQ(v, 0xbeefu);
}
