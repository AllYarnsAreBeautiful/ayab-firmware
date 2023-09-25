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

using ::testing::Return;

extern Encoders *encoders;

class EncodersTest : public ::testing::Test {
protected:
  void SetUp() override {
    arduinoMock = arduinoMockInstance();
    encoders->init(Machine_t::Kh910);
  }

  void TearDown() override {
    releaseArduinoMock();
  }

  ArduinoMock *arduinoMock;
};

TEST_F(EncodersTest, test_encA_rising_not_before) {
  // We should not enter the falling function
  EXPECT_CALL(*arduinoMock, analogRead(EOL_PIN_R)).Times(0);
  // Create a rising edge
  EXPECT_CALL(*arduinoMock, digitalRead(ENC_PIN_A))
      .WillOnce(Return(false))
      .WillOnce(Return(true));
  // We have not entered the rising function yet
  EXPECT_CALL(*arduinoMock, analogRead(EOL_PIN_L)).Times(0);
  encoders->isr();

  // Enter rising function, direction is Right
  EXPECT_CALL(*arduinoMock, digitalRead(ENC_PIN_B)).WillOnce(Return(true));
  // Not before Left Hall Sensor
  EXPECT_CALL(*arduinoMock, analogRead(EOL_PIN_L))
      .WillOnce(Return(FILTER_L_MIN[static_cast<int8_t>(encoders->getMachineType())]));
  encoders->isr();

  ASSERT_EQ(encoders->getDirection(), Direction_t::Right);
  ASSERT_EQ(encoders->getPosition(), 0x01);
  ASSERT_EQ(encoders->getCarriage(), Carriage_t::NoCarriage);
}

TEST_F(EncodersTest, test_encA_rising_in_front_notKH270) {
  ASSERT_FALSE(encoders->getMachineType() == Machine_t::Kh270);
  ASSERT_EQ(encoders->getCarriage(), Carriage_t::NoCarriage);

  // We should not enter the falling function
  EXPECT_CALL(*arduinoMock, analogRead(EOL_PIN_R)).Times(0);
  // Create a rising edge
  EXPECT_CALL(*arduinoMock, digitalRead(ENC_PIN_A)).WillOnce(Return(false));
  // We have not entered the rising function yet
  EXPECT_CALL(*arduinoMock, analogRead(EOL_PIN_L)).Times(0);
  encoders->isr();

  // Create a rising edge
  EXPECT_CALL(*arduinoMock, digitalRead(ENC_PIN_A)).WillOnce(Return(true));
  // Enter rising function, direction is Right
  EXPECT_CALL(*arduinoMock, digitalRead(ENC_PIN_B)).WillOnce(Return(true));
  // Before of Left Hall Sensor
  EXPECT_CALL(*arduinoMock, analogRead(EOL_PIN_L))
      .WillOnce(Return(FILTER_L_MIN[static_cast<int8_t>(encoders->getMachineType())] - 1));
  // BeltShift is regular
  EXPECT_CALL(*arduinoMock, digitalRead(ENC_PIN_C)).WillOnce(Return(true));
  encoders->isr();

  ASSERT_EQ(encoders->getDirection(), Direction_t::Right);
  ASSERT_EQ(encoders->getHallActive(), Direction_t::Left);
  ASSERT_EQ(encoders->getPosition(), END_LEFT_PLUS_OFFSET[static_cast<int8_t>(encoders->getMachineType())]);
  ASSERT_EQ(encoders->getCarriage(), Carriage_t::Lace);
  ASSERT_EQ(encoders->getBeltShift(), BeltShift::Regular);
}

TEST_F(EncodersTest, test_encA_rising_before_KH270) {
  encoders->init(Machine_t::Kh270);
  ASSERT_TRUE(encoders->getMachineType() == Machine_t::Kh270);

  // We should not enter the falling function
  EXPECT_CALL(*arduinoMock, analogRead(EOL_PIN_R)).Times(0);
  // Create a rising edge
  EXPECT_CALL(*arduinoMock, digitalRead(ENC_PIN_A)).WillOnce(Return(false));
  // We have not entered the rising function yet
  EXPECT_CALL(*arduinoMock, analogRead(EOL_PIN_L)).Times(0);
  encoders->isr();

  // Create a rising edge
  EXPECT_CALL(*arduinoMock, digitalRead(ENC_PIN_A)).WillOnce(Return(true));
  // Enter rising function, direction is Right
  EXPECT_CALL(*arduinoMock, digitalRead(ENC_PIN_B)).WillOnce(Return(true));
  // Before Left Hall Sensor
  EXPECT_CALL(*arduinoMock, analogRead(EOL_PIN_L))
      .WillOnce(Return(FILTER_L_MIN[static_cast<int8_t>(Machine_t::Kh270)] - 1));
  // BeltShift is regular
  EXPECT_CALL(*arduinoMock, digitalRead(ENC_PIN_C)).WillOnce(Return(true));
  encoders->isr();

  ASSERT_EQ(encoders->getDirection(), Direction_t::Right);
  ASSERT_EQ(encoders->getHallActive(), Direction_t::Left);
  ASSERT_EQ(encoders->getPosition(), END_LEFT_PLUS_OFFSET[static_cast<int8_t>(Machine_t::Kh270)]);
  ASSERT_EQ(encoders->getCarriage(), Carriage_t::Knit);
  ASSERT_EQ(encoders->getBeltShift(), BeltShift::Regular);

  // Create a falling edge, then a rising edge:
  EXPECT_CALL(*arduinoMock, digitalRead(ENC_PIN_A)).WillOnce(Return(false)).WillOnce(Return(true));
  EXPECT_CALL(*arduinoMock, digitalRead(ENC_PIN_B)).WillOnce(Return(false)).WillOnce(Return(true));
  // Not before Left Hall Sensor
  EXPECT_CALL(*arduinoMock, analogRead(EOL_PIN_R))
      .WillOnce(Return(FILTER_R_MIN[static_cast<int8_t>(encoders->getMachineType())]));
  // We will not enter the rising function
  EXPECT_CALL(*arduinoMock, analogRead(EOL_PIN_L)).Times(0);
  encoders->isr();
  encoders->isr();
}

TEST_F(EncodersTest, test_encA_rising_after_KH270) {
  encoders->init(Machine_t::Kh270);
  ASSERT_TRUE(encoders->getMachineType() == Machine_t::Kh270);

  // We should not enter the falling function
  EXPECT_CALL(*arduinoMock, analogRead(EOL_PIN_R)).Times(0);
  // Create a rising edge
  EXPECT_CALL(*arduinoMock, digitalRead(ENC_PIN_A)).WillOnce(Return(false));
  // We have not entered the rising function yet
  EXPECT_CALL(*arduinoMock, analogRead(EOL_PIN_L)).Times(0);
  encoders->isr();

  // Create a rising edge
  EXPECT_CALL(*arduinoMock, digitalRead(ENC_PIN_A)).WillOnce(Return(true));
  // Enter rising function, direction is Right
  EXPECT_CALL(*arduinoMock, digitalRead(ENC_PIN_B)).WillOnce(Return(true));
  // After Left Hall Sensor
  EXPECT_CALL(*arduinoMock, analogRead(EOL_PIN_L))
      .WillOnce(Return(FILTER_L_MAX[static_cast<int8_t>(encoders->getMachineType())] + 1));
  // BeltShift is regular
  EXPECT_CALL(*arduinoMock, digitalRead(ENC_PIN_C)).WillOnce(Return(true));
  encoders->isr();

  ASSERT_EQ(encoders->getDirection(), Direction_t::Right);
  ASSERT_EQ(encoders->getHallActive(), Direction_t::Left);
  ASSERT_EQ(encoders->getPosition(), END_LEFT_PLUS_OFFSET[static_cast<int8_t>(Machine_t::Kh270)] + MAGNET_DISTANCE_270);
  ASSERT_EQ(encoders->getCarriage(), Carriage_t::Knit);
  ASSERT_EQ(encoders->getBeltShift(), BeltShift::Regular);
}

TEST_F(EncodersTest, test_encA_rising_in_front_G_carriage) {
  // Create a rising edge
  EXPECT_CALL(*arduinoMock, digitalRead(ENC_PIN_A)).WillOnce(Return(true));
  // Enter rising function, direction is Right
  EXPECT_CALL(*arduinoMock, digitalRead(ENC_PIN_B)).WillOnce(Return(true));
  // In front of Left Hall Sensor
  EXPECT_CALL(*arduinoMock, analogRead(EOL_PIN_L))
      .WillOnce(Return(FILTER_L_MAX[static_cast<int8_t>(encoders->getMachineType())] + 1));
  // BeltShift is regular
  EXPECT_CALL(*arduinoMock, digitalRead(ENC_PIN_C)).WillOnce(Return(true));
  encoders->isr();

  ASSERT_EQ(encoders->getCarriage(), Carriage_t::Knit);

  // Create a falling edge
  EXPECT_CALL(*arduinoMock, digitalRead(ENC_PIN_A)).WillOnce(Return(false));
  // Enter falling function, direction is Right
  EXPECT_CALL(*arduinoMock, digitalRead(ENC_PIN_B)).WillOnce(Return(false));
  EXPECT_CALL(*arduinoMock, analogRead(EOL_PIN_R))
      .WillOnce(Return(FILTER_R_MAX[static_cast<int8_t>(encoders->getMachineType())] + 1));
  EXPECT_CALL(*arduinoMock, digitalRead(ENC_PIN_C));
  encoders->isr();

  // Create a rising edge
  EXPECT_CALL(*arduinoMock, digitalRead(ENC_PIN_A)).WillOnce(Return(true));
  // Enter rising function, direction is Right
  EXPECT_CALL(*arduinoMock, digitalRead(ENC_PIN_B)).WillOnce(Return(true));
  // In front of Left Hall Sensor
  EXPECT_CALL(*arduinoMock, analogRead(EOL_PIN_L))
      .WillOnce(Return(FILTER_L_MIN[static_cast<int8_t>(encoders->getMachineType())] - 1));
  encoders->isr();

  ASSERT_EQ(encoders->getCarriage(), Carriage_t::Garter);

  // Create a falling edge, then a rising edge:
  EXPECT_CALL(*arduinoMock, digitalRead(ENC_PIN_A)).WillOnce(Return(false)).WillOnce(Return(true));
  EXPECT_CALL(*arduinoMock, digitalRead(ENC_PIN_B)).WillOnce(Return(false)).WillOnce(Return(true));
  EXPECT_CALL(*arduinoMock, analogRead(EOL_PIN_R))
      .WillOnce(Return(FILTER_R_MIN[static_cast<int8_t>(encoders->getMachineType())] + 1));
  // We will not enter the rising function
  EXPECT_CALL(*arduinoMock, analogRead(EOL_PIN_L)).Times(0);
  encoders->isr();
  encoders->isr();
}

TEST_F(EncodersTest, test_encA_falling_not_in_front) {
  // Rising edge, direction is Right
  EXPECT_CALL(*arduinoMock, digitalRead(ENC_PIN_A)).WillOnce(Return(true));
  EXPECT_CALL(*arduinoMock, digitalRead(ENC_PIN_B)).WillOnce(Return(true));
  // Not in front of Left Hall Sensor
  EXPECT_CALL(*arduinoMock, analogRead(EOL_PIN_L))
      .WillOnce(Return(FILTER_L_MIN[static_cast<int8_t>(encoders->getMachineType())]));
  encoders->isr();

  // Rising edge, function not entered
  EXPECT_CALL(*arduinoMock, digitalRead(ENC_PIN_A)).WillOnce(Return(true));
  EXPECT_CALL(*arduinoMock, digitalRead(ENC_PIN_B)).Times(0);
  EXPECT_CALL(*arduinoMock, analogRead(EOL_PIN_L)).Times(0);
  encoders->isr();
/*
  // Falling edge, direction is Left
  encoders->m_position = END_LEFT[static_cast<uint8_t>(encoders->getMachineType())] + 1;
  EXPECT_CALL(*arduinoMock, digitalRead(ENC_PIN_A)).WillOnce(Return(false));
  EXPECT_CALL(*arduinoMock, digitalRead(ENC_PIN_B)).WillOnce(Return(false));
  EXPECT_CALL(*arduinoMock, analogRead(EOL_PIN_R))
      .WillOnce(Return(FILTER_R_MIN[static_cast<int8_t>(encoders->getMachineType())]));
  encoders->isr();
*/
}

TEST_F(EncodersTest, test_encA_falling_in_front) {
  // Enter rising function, direction is Left
  EXPECT_CALL(*arduinoMock, digitalRead(ENC_PIN_A)).WillOnce(Return(true));
  EXPECT_CALL(*arduinoMock, digitalRead(ENC_PIN_B)).WillOnce(Return(false));
  // In front of Left Hall Sensor
  EXPECT_CALL(*arduinoMock, analogRead(EOL_PIN_L))
      .WillOnce(Return(FILTER_L_MIN[static_cast<int8_t>(encoders->getMachineType())]));
  encoders->isr();

  // Falling edge, direction is Left
  EXPECT_CALL(*arduinoMock, digitalRead(ENC_PIN_A)).WillOnce(Return(false));
  EXPECT_CALL(*arduinoMock, digitalRead(ENC_PIN_B)).WillOnce(Return(false));
  EXPECT_CALL(*arduinoMock, analogRead(EOL_PIN_R))
      .WillOnce(Return(FILTER_R_MAX[static_cast<int8_t>(encoders->getMachineType())] + 1));
  // BeltShift is shifted
  EXPECT_CALL(*arduinoMock, digitalRead(ENC_PIN_C)).WillOnce(Return(true));
  encoders->isr();

  ASSERT_EQ(encoders->getDirection(), Direction_t::Right);
  ASSERT_EQ(encoders->getHallActive(), Direction_t::Right);
  ASSERT_EQ(encoders->getPosition(), END_RIGHT_MINUS_OFFSET[static_cast<int8_t>(encoders->getMachineType())]);
  ASSERT_EQ(encoders->getCarriage(), Carriage_t::NoCarriage);
  ASSERT_EQ(encoders->getBeltShift(), BeltShift::Shifted);
}

TEST_F(EncodersTest, test_encA_falling_at_end) {
  // Rising edge, direction is Right
  EXPECT_CALL(*arduinoMock, digitalRead(ENC_PIN_A)).WillOnce(Return(true));
  EXPECT_CALL(*arduinoMock, digitalRead(ENC_PIN_B)).WillOnce(Return(false));
  // In front of Left Hall Sensor
  EXPECT_CALL(*arduinoMock, analogRead(EOL_PIN_L))
      .WillOnce(Return(FILTER_L_MAX[static_cast<int8_t>(encoders->getMachineType())]));
  EXPECT_CALL(*arduinoMock, digitalRead(ENC_PIN_C)).Times(0);
  encoders->isr();

  // Falling edge, direction is Left
  EXPECT_CALL(*arduinoMock, digitalRead(ENC_PIN_A)).WillOnce(Return(false));
  EXPECT_CALL(*arduinoMock, digitalRead(ENC_PIN_B)).WillOnce(Return(true));
  EXPECT_CALL(*arduinoMock, analogRead(EOL_PIN_R))
      .WillOnce(Return(FILTER_R_MAX[static_cast<int8_t>(encoders->getMachineType())] + 1));
  EXPECT_CALL(*arduinoMock, digitalRead(ENC_PIN_C));
  encoders->isr();

  ASSERT_EQ(encoders->getPosition(), END_RIGHT_MINUS_OFFSET[static_cast<int8_t>(encoders->getMachineType())]);

  uint16_t pos = END_RIGHT_MINUS_OFFSET[static_cast<int8_t>(encoders->getMachineType())];
  while (pos < END_RIGHT[static_cast<int8_t>(encoders->getMachineType())]) {
    // Rising edge
    EXPECT_CALL(*arduinoMock, digitalRead(ENC_PIN_A)).WillOnce(Return(true));
    EXPECT_CALL(*arduinoMock, digitalRead(ENC_PIN_B)).WillOnce(Return(true));
    EXPECT_CALL(*arduinoMock, analogRead(EOL_PIN_L))
        .WillOnce(Return(FILTER_L_MAX[static_cast<int8_t>(encoders->getMachineType())]));
    encoders->isr();

    ASSERT_EQ(encoders->getPosition(), ++pos);

    // Falling edge
    EXPECT_CALL(*arduinoMock, digitalRead(ENC_PIN_A)).WillOnce(Return(false));
    EXPECT_CALL(*arduinoMock, digitalRead(ENC_PIN_B));
    EXPECT_CALL(*arduinoMock, analogRead(EOL_PIN_R))
        .WillOnce(Return(FILTER_R_MAX[static_cast<int8_t>(encoders->getMachineType())]));
    encoders->isr();

    ASSERT_EQ(encoders->getPosition(), pos);
  }

  ASSERT_EQ(encoders->getPosition(), pos);
  // Rising edge
  EXPECT_CALL(*arduinoMock, digitalRead(ENC_PIN_A)).WillOnce(Return(true));
  EXPECT_CALL(*arduinoMock, digitalRead(ENC_PIN_B)).WillOnce(Return(true));
  EXPECT_CALL(*arduinoMock, analogRead(EOL_PIN_L))
      .WillOnce(Return(FILTER_L_MAX[static_cast<int8_t>(encoders->getMachineType())]));
  encoders->isr();

  ASSERT_EQ(encoders->getPosition(), pos);
}

// requires FILTER_R_MIN != 0
TEST_F(EncodersTest, test_encA_falling_set_K_carriage_KH910) {
  ASSERT_TRUE(encoders->getMachineType() == Machine_t::Kh910);

  // Create a rising edge
  EXPECT_CALL(*arduinoMock, digitalRead(ENC_PIN_A)).WillOnce(Return(true));
  EXPECT_CALL(*arduinoMock, digitalRead(ENC_PIN_B));
  EXPECT_CALL(*arduinoMock, digitalRead(ENC_PIN_C));
  EXPECT_CALL(*arduinoMock, analogRead(EOL_PIN_L));
  encoders->isr();

  // falling edge
  EXPECT_CALL(*arduinoMock, digitalRead(ENC_PIN_A)).WillOnce(Return(false));
  EXPECT_CALL(*arduinoMock, digitalRead(ENC_PIN_B));
  EXPECT_CALL(*arduinoMock, analogRead(EOL_PIN_R))
      .WillOnce(Return(FILTER_R_MIN[static_cast<int8_t>(encoders->getMachineType())] - 1));
  EXPECT_CALL(*arduinoMock, digitalRead(ENC_PIN_C));
  encoders->isr();

  ASSERT_EQ(encoders->getCarriage(), Carriage_t::Knit);
}

TEST_F(EncodersTest, test_encA_falling_not_at_end) {
  // rising edge, direction is Left
  EXPECT_CALL(*arduinoMock, digitalRead(ENC_PIN_A)).WillOnce(Return(true));
  EXPECT_CALL(*arduinoMock, digitalRead(ENC_PIN_B)).WillOnce(Return(false));
  EXPECT_CALL(*arduinoMock, digitalRead(ENC_PIN_C));
  EXPECT_CALL(*arduinoMock, analogRead(EOL_PIN_L))
      .WillOnce(Return(FILTER_R_MAX[static_cast<int8_t>(encoders->getMachineType())] + 1));
  encoders->isr();

  ASSERT_EQ(encoders->getPosition(), END_LEFT_PLUS_OFFSET[static_cast<int8_t>(encoders->getMachineType())]);

  // falling edge, direction is Left, and pos is > 0
  EXPECT_CALL(*arduinoMock, digitalRead(ENC_PIN_A)).WillOnce(Return(false));
  EXPECT_CALL(*arduinoMock, digitalRead(ENC_PIN_B)).WillOnce(Return(false));
  EXPECT_CALL(*arduinoMock, analogRead(EOL_PIN_R))
      .WillOnce(Return(FILTER_R_MAX[static_cast<int8_t>(encoders->getMachineType())]));
  encoders->isr();

  ASSERT_EQ(encoders->getPosition(), END_LEFT_PLUS_OFFSET[static_cast<int8_t>(encoders->getMachineType())]);
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
