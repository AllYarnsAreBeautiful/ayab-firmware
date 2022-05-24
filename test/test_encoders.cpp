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

using ::testing::Return;

extern Encoders *encoders;

class EncodersTest : public ::testing::Test {
protected:
  void SetUp() override {
    arduinoMock = arduinoMockInstance();
    encoders->init(Kh910);
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
      .WillOnce(Return(FILTER_L_MIN[encoders->getMachineType()]));
  encoders->encA_interrupt();
  ASSERT_EQ(encoders->getDirection(), Right);
  ASSERT_EQ(encoders->getPosition(), 0x01);
  ASSERT_EQ(encoders->getCarriage(), NoCarriage);
}

TEST_F(EncodersTest, test_encA_rising_in_front_notKH270) {
  ASSERT_FALSE(encoders->getMachineType() == Kh270);
  ASSERT_EQ(encoders->getCarriage(), NoCarriage);
  // We should not enter the falling function
  EXPECT_CALL(*arduinoMock, analogRead(EOL_PIN_R)).Times(0);
  // Create a rising edge
  EXPECT_CALL(*arduinoMock, digitalRead(ENC_PIN_A)).WillOnce(Return(false));
  // We have not entered the rising function yet
  EXPECT_CALL(*arduinoMock, analogRead(EOL_PIN_L)).Times(0);

  encoders->encA_interrupt();

  // Create a rising edge
  EXPECT_CALL(*arduinoMock, digitalRead(ENC_PIN_A)).WillOnce(Return(true));
  // Enter rising function, direction is right
  EXPECT_CALL(*arduinoMock, digitalRead(ENC_PIN_B)).WillOnce(Return(true));
  // In front of Left Hall Sensor
  EXPECT_CALL(*arduinoMock, analogRead(EOL_PIN_L))
      .WillOnce(Return(FILTER_L_MIN[encoders->getMachineType()] - 1));
  // BeltShift is regular
  EXPECT_CALL(*arduinoMock, digitalRead(ENC_PIN_C)).WillOnce(Return(true));

  encoders->encA_interrupt();

  ASSERT_EQ(encoders->getDirection(), Right);
  ASSERT_EQ(encoders->getHallActive(), Left);
  ASSERT_EQ(encoders->getPosition(), END_OFFSET[encoders->getMachineType()]);
  ASSERT_EQ(encoders->getCarriage(), Lace);
  ASSERT_EQ(encoders->getBeltShift(), Regular);
}

TEST_F(EncodersTest, test_encA_rising_in_front_KH270) {
  encoders->init(Kh270);
  ASSERT_TRUE(encoders->getMachineType() == Kh270);
  // We should not enter the falling function
  EXPECT_CALL(*arduinoMock, analogRead(EOL_PIN_R)).Times(0);
  // Create a rising edge
  EXPECT_CALL(*arduinoMock, digitalRead(ENC_PIN_A)).WillOnce(Return(false));
  // We have not entered the rising function yet
  EXPECT_CALL(*arduinoMock, analogRead(EOL_PIN_L)).Times(0);

  encoders->encA_interrupt();

  // Create a rising edge
  EXPECT_CALL(*arduinoMock, digitalRead(ENC_PIN_A)).WillOnce(Return(true));
  // Enter rising function, direction is right
  EXPECT_CALL(*arduinoMock, digitalRead(ENC_PIN_B)).WillOnce(Return(true));
  // In front of Left Hall Sensor
  EXPECT_CALL(*arduinoMock, analogRead(EOL_PIN_L))
      .WillOnce(Return(FILTER_L_MIN[encoders->getMachineType()] - 1));
  // BeltShift is regular
  EXPECT_CALL(*arduinoMock, digitalRead(ENC_PIN_C)).WillOnce(Return(true));

  encoders->encA_interrupt();

  ASSERT_EQ(encoders->getDirection(), Right);
  ASSERT_EQ(encoders->getHallActive(), Left);
  ASSERT_EQ(encoders->getPosition(), END_OFFSET[encoders->getMachineType()]);
  ASSERT_EQ(encoders->getCarriage(), Knit);
  ASSERT_EQ(encoders->getBeltShift(), Regular);
}

TEST_F(EncodersTest, test_encA_rising_in_front_G_carriage) {
  // Create a rising edge
  EXPECT_CALL(*arduinoMock, digitalRead(ENC_PIN_A)).WillOnce(Return(true));
  // Enter rising function, direction is right
  EXPECT_CALL(*arduinoMock, digitalRead(ENC_PIN_B)).WillOnce(Return(true));
  // In front of Left Hall Sensor
  EXPECT_CALL(*arduinoMock, analogRead(EOL_PIN_L))
      .WillOnce(Return(FILTER_L_MAX[encoders->getMachineType()] + 1));
  // BeltShift is regular
  EXPECT_CALL(*arduinoMock, digitalRead(ENC_PIN_C)).WillOnce(Return(true));

  encoders->encA_interrupt();

  ASSERT_EQ(encoders->getCarriage(), Knit);

  // Create a falling edge
  EXPECT_CALL(*arduinoMock, digitalRead(ENC_PIN_A)).WillOnce(Return(false));
  EXPECT_CALL(*arduinoMock, analogRead(EOL_PIN_R))
      .WillOnce(Return(FILTER_R_MAX[encoders->getMachineType()] + 1));
  EXPECT_CALL(*arduinoMock, digitalRead(ENC_PIN_C));
  encoders->encA_interrupt();
  // Create a rising edge
  EXPECT_CALL(*arduinoMock, digitalRead(ENC_PIN_A)).WillOnce(Return(true));
  // Enter rising function, direction is right
  EXPECT_CALL(*arduinoMock, digitalRead(ENC_PIN_B)).WillOnce(Return(true));
  // In front of Left Hall Sensor
  EXPECT_CALL(*arduinoMock, analogRead(EOL_PIN_L))
      .WillOnce(Return(FILTER_L_MIN[encoders->getMachineType()] - 1));

  encoders->encA_interrupt();

  ASSERT_EQ(encoders->getCarriage(), Garter);
}

TEST_F(EncodersTest, test_encA_falling_not_in_front) {
  // Create a falling edge
  EXPECT_CALL(*arduinoMock, digitalRead(ENC_PIN_A))
      .WillOnce(Return(true))
      .WillOnce(Return(true));
  // We have not entered the falling function yet
  EXPECT_CALL(*arduinoMock, analogRead(EOL_PIN_R)).Times(0);

  // Enter rising function, direction is right
  EXPECT_CALL(*arduinoMock, digitalRead(ENC_PIN_B)).WillOnce(Return(true));
  // Not in front of Left Hall Sensor
  EXPECT_CALL(*arduinoMock, analogRead(EOL_PIN_L))
      .WillOnce(Return(FILTER_L_MIN[encoders->getMachineType()]));
  encoders->encA_interrupt();
  encoders->encA_interrupt();

  EXPECT_CALL(*arduinoMock, digitalRead(ENC_PIN_A)).WillOnce(Return(false));
  EXPECT_CALL(*arduinoMock, analogRead(EOL_PIN_R))
      .WillOnce(Return(FILTER_R_MIN[encoders->getMachineType()]));

  encoders->encA_interrupt();
}

TEST_F(EncodersTest, test_encA_falling_in_front) {
  // Create a falling edge
  EXPECT_CALL(*arduinoMock, digitalRead(ENC_PIN_A))
      .WillOnce(Return(true))
      .WillOnce(Return(true));
  // We have not entered the falling function yet
  EXPECT_CALL(*arduinoMock, analogRead(EOL_PIN_R)).Times(0);

  // Enter rising function, direction is left
  EXPECT_CALL(*arduinoMock, digitalRead(ENC_PIN_B)).WillOnce(Return(false));
  // In front of Left Hall Sensor
  EXPECT_CALL(*arduinoMock, analogRead(EOL_PIN_L))
      .WillOnce(Return(FILTER_L_MIN[encoders->getMachineType()]));
  encoders->encA_interrupt();
  encoders->encA_interrupt();

  EXPECT_CALL(*arduinoMock, digitalRead(ENC_PIN_A)).WillOnce(Return(false));
  EXPECT_CALL(*arduinoMock, analogRead(EOL_PIN_R))
      .WillOnce(Return(FILTER_R_MAX[encoders->getMachineType()] + 1));
  // BeltShift is shifted
  EXPECT_CALL(*arduinoMock, digitalRead(ENC_PIN_C)).WillOnce(Return(true));

  encoders->encA_interrupt();

  ASSERT_EQ(encoders->getDirection(), Left);
  ASSERT_EQ(encoders->getHallActive(), Right);
  ASSERT_EQ(encoders->getPosition(), 227);
  ASSERT_EQ(encoders->getCarriage(), NoCarriage);
  ASSERT_EQ(encoders->getBeltShift(), Shifted);
}

TEST_F(EncodersTest, test_encA_falling_at_end) {
  // Create a falling edge
  EXPECT_CALL(*arduinoMock, digitalRead(ENC_PIN_A)).WillOnce(Return(true));

  // Enter rising function, direction is left
  EXPECT_CALL(*arduinoMock, digitalRead(ENC_PIN_B)).WillOnce(Return(false));
  // In front of Left Hall Sensor
  EXPECT_CALL(*arduinoMock, analogRead(EOL_PIN_L))
      .WillOnce(Return(FILTER_L_MAX[encoders->getMachineType()]));
  EXPECT_CALL(*arduinoMock, digitalRead(ENC_PIN_C)).Times(0);
  encoders->encA_interrupt();

  EXPECT_CALL(*arduinoMock, digitalRead(ENC_PIN_A)).WillOnce(Return(false));
  EXPECT_CALL(*arduinoMock, analogRead(EOL_PIN_R))
      .WillOnce(Return(FILTER_R_MAX[encoders->getMachineType()] + 1));
  EXPECT_CALL(*arduinoMock, digitalRead(ENC_PIN_C));

  encoders->encA_interrupt();
  ASSERT_EQ(encoders->getPosition(), 227);

  uint16_t pos = 227;
  while (pos < END_RIGHT[encoders->getMachineType()]) {
    // Rising
    EXPECT_CALL(*arduinoMock, digitalRead(ENC_PIN_A)).WillOnce(Return(true));
    EXPECT_CALL(*arduinoMock, digitalRead(ENC_PIN_B)).WillOnce(Return(true));
    EXPECT_CALL(*arduinoMock, analogRead(EOL_PIN_L))
        .WillOnce(Return(FILTER_L_MAX[encoders->getMachineType()]));
    encoders->encA_interrupt();
    ASSERT_EQ(encoders->getPosition(), ++pos);

    // Falling
    EXPECT_CALL(*arduinoMock, digitalRead(ENC_PIN_A)).WillOnce(Return(false));
    EXPECT_CALL(*arduinoMock, analogRead(EOL_PIN_R))
        .WillOnce(Return(FILTER_R_MAX[encoders->getMachineType()]));
    encoders->encA_interrupt();
    ASSERT_EQ(encoders->getPosition(), pos);
  }

  ASSERT_EQ(encoders->getPosition(), pos);
  // Rising
  EXPECT_CALL(*arduinoMock, digitalRead(ENC_PIN_A)).WillOnce(Return(true));
  EXPECT_CALL(*arduinoMock, digitalRead(ENC_PIN_B)).WillOnce(Return(true));
  EXPECT_CALL(*arduinoMock, analogRead(EOL_PIN_L))
      .WillOnce(Return(FILTER_L_MAX[encoders->getMachineType()]));
  encoders->encA_interrupt();
  ASSERT_EQ(encoders->getPosition(), pos);
}

// requires FILTER_R_MIN != 0
TEST_F(EncodersTest, test_encA_falling_set_K_carriage_KH910) {
  ASSERT_TRUE(encoders->getMachineType() == Kh910);

  // Create a rising edge
  EXPECT_CALL(*arduinoMock, digitalRead(ENC_PIN_A)).WillOnce(Return(true));
  EXPECT_CALL(*arduinoMock, digitalRead(ENC_PIN_B));
  EXPECT_CALL(*arduinoMock, digitalRead(ENC_PIN_C));
  EXPECT_CALL(*arduinoMock, analogRead(EOL_PIN_L));
  encoders->encA_interrupt();

  // falling edge
  EXPECT_CALL(*arduinoMock, digitalRead(ENC_PIN_A)).WillOnce(Return(false));
  EXPECT_CALL(*arduinoMock, analogRead(EOL_PIN_R))
      .WillOnce(Return(FILTER_R_MIN[encoders->getMachineType()] - 1));
  EXPECT_CALL(*arduinoMock, digitalRead(ENC_PIN_C));

  encoders->encA_interrupt();
  ASSERT_EQ(encoders->getCarriage(), Knit);
}

TEST_F(EncodersTest, test_encA_falling_not_at_end) {
  // rising, direction is left
  EXPECT_CALL(*arduinoMock, digitalRead(ENC_PIN_A)).WillOnce(Return(true));
  EXPECT_CALL(*arduinoMock, digitalRead(ENC_PIN_B)).WillOnce(Return(false));
  EXPECT_CALL(*arduinoMock, digitalRead(ENC_PIN_C));
  EXPECT_CALL(*arduinoMock, analogRead(EOL_PIN_L))
      .WillOnce(Return(FILTER_R_MAX[encoders->getMachineType()] + 1));
  encoders->encA_interrupt();
  ASSERT_EQ(encoders->getPosition(), 28);

  // falling, direction is left and pos is > 0
  EXPECT_CALL(*arduinoMock, digitalRead(ENC_PIN_A)).WillOnce(Return(false));
  EXPECT_CALL(*arduinoMock, analogRead(EOL_PIN_R))
      .WillOnce(Return(FILTER_R_MAX[encoders->getMachineType()]));
  encoders->encA_interrupt();
  ASSERT_EQ(encoders->getPosition(), 27);
}

TEST_F(EncodersTest, test_getPosition) {
  uint8_t p = encoders->getPosition();
  ASSERT_EQ(p, 0x00);
}

TEST_F(EncodersTest, test_getBeltShift) {
  BeltShift_t b = encoders->getBeltShift();
  ASSERT_EQ(b, Unknown);
}

TEST_F(EncodersTest, test_getDirection) {
  Direction_t d = encoders->getDirection();
  ASSERT_EQ(d, NoDirection);
}

TEST_F(EncodersTest, test_getHallActive) {
  Direction_t d = encoders->getHallActive();
  ASSERT_EQ(d, NoDirection);
}

TEST_F(EncodersTest, test_getCarriage) {
  Carriage_t c = encoders->getCarriage();
  ASSERT_EQ(c, NoCarriage);
}

TEST_F(EncodersTest, test_getMachineType) {
  Machine_t m = encoders->getMachineType();
  ASSERT_EQ(m, Kh910);
}

TEST_F(EncodersTest, test_init) {
  encoders->init(Kh270);
  Machine_t m = encoders->getMachineType();
  ASSERT_EQ(m, Kh270);
}

TEST_F(EncodersTest, test_getHallValue) {
  uint16_t v = encoders->getHallValue(NoDirection);
  ASSERT_EQ(v, 0u);
  EXPECT_CALL(*arduinoMock, analogRead(EOL_PIN_L));
  v = encoders->getHallValue(Left);
  ASSERT_EQ(v, 0u);
  EXPECT_CALL(*arduinoMock, analogRead(EOL_PIN_R));
  v = encoders->getHallValue(Right);
  ASSERT_EQ(v, 0u);
  EXPECT_CALL(*arduinoMock, analogRead(EOL_PIN_R)).WillOnce(Return(0xbeefu));
  v = encoders->getHallValue(Right);
  ASSERT_EQ(v, 0xbeefu);
}
