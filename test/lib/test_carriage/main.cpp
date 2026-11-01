#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <initializer_list>

#include "carriage.h"
#include "encoder.h"
#include "hallsensor.h"
#include "mocks/hal_mock.h"

using ::testing::NiceMock;
using ::testing::Return;
using ::testing::ReturnPointee;

namespace {

constexpr uint8_t kSensorPin = 14;
constexpr uint8_t kSensorPin2 = 7;
constexpr uint8_t kDetectPin = 8;
constexpr uint8_t kEncoderPinA = 2;
constexpr uint8_t kEncoderPinB = 3;

class CarriageDetectionTest : public ::testing::Test {
 protected:
  void SetUp() override {
    hal = new NiceMock<Hal_mock>();
    ON_CALL(*hal, analogRead(kSensorPin))
        .WillByDefault(ReturnPointee(&sensorValue));

    encoder = new Encoder(hal, kEncoderPinA, kEncoderPinB);
    sensor = new HallSensor(hal, kSensorPin);
    sensor->config(&config);
  }

  void TearDown() override {
    delete sensor;
    delete encoder;
    delete hal;
  }

  bool sample(int16_t position, uint16_t value, Direction direction) {
    encoder->setPosition(position);
    sensorValue = value;
    return sensor->isDetected(encoder, direction, false);
  }

  bool detect(std::initializer_list<uint16_t> values,
              Direction direction = Direction::Right) {
    int16_t position = 0;
    bool detected = false;
    for (uint16_t value : values) {
      detected = sample(position, value, direction);
      position += direction == Direction::Right ? 1 : -1;
    }
    return detected;
  }

  void expectDetected(CarriageType expected,
                      std::initializer_list<uint16_t> values,
                      Direction direction = Direction::Right) {
    ASSERT_TRUE(detect(values, direction));
    EXPECT_EQ(sensor->getDetectedCarriage(), expected);
  }

  void crossCarriage(Carriage &carriage, CarriageType expected,
                     std::initializer_list<uint16_t> values,
                     Direction direction = Direction::Right) {
    expectDetected(expected, values, direction);
    carriage.setPosition(encoder->getPosition());
    ASSERT_TRUE(carriage.isCrossing(sensor, direction));
    ASSERT_EQ(carriage.getType(), expected);
  }

  NiceMock<Hal_mock> *hal = nullptr;
  Encoder *encoder = nullptr;
  HallSensor *sensor = nullptr;
  uint16_t sensorValue = 512;
  HallSensor::Config config = {
      .position = 0,
      .thresholdLow = 200,
      .thresholdHigh = 600,
      .flags = 0,
  };
};

TEST_F(CarriageDetectionTest, DetectsKnitCarriageFromNorthPole) {
  expectDetected(CarriageType::Knit, {800, 800, 512, 512});
  EXPECT_EQ(sensor->getDetectedPosition(), 0);
}

TEST_F(CarriageDetectionTest, DetectsLaceCarriageFromSouthPole) {
  expectDetected(CarriageType::Lace, {100, 100, 512, 512});
  EXPECT_EQ(sensor->getDetectedPosition(), 0);
}

TEST_F(CarriageDetectionTest, DetectsGarterCarriageFromPolePair) {
  expectDetected(CarriageType::Garter, {100, 800, 512, 512});
  EXPECT_EQ(sensor->getDetectedPosition(), 0);
}

TEST_F(CarriageDetectionTest, DetectsKh270KnitCarriage) {
  config.flags = HALLSENSOR_K270_K;
  sensor->config(&config);

  expectDetected(CarriageType::Knit270, {800, 800, 512, 512});
}

TEST_F(CarriageDetectionTest, DetectsEveryCarriageMovingLeft) {
  struct DetectionCase {
    CarriageType type;
    std::initializer_list<uint16_t> sensorValues;
    uint8_t flags;
  };

  const DetectionCase cases[] = {
      {CarriageType::Knit, {800, 800, 512, 512}, 0},
      {CarriageType::Lace, {100, 100, 512, 512}, 0},
      {CarriageType::Garter, {100, 800, 512, 512}, 0},
      {CarriageType::Knit270,
       {800, 800, 512, 512},
       HALLSENSOR_K270_K},
  };

  for (const DetectionCase &detectionCase : cases) {
    config.flags = detectionCase.flags;
    sensor->config(&config);
    expectDetected(detectionCase.type, detectionCase.sensorValues,
                   Direction::Left);
  }
}

TEST_F(CarriageDetectionTest, RejectsDetectionAfterDirectionChange) {
  EXPECT_FALSE(sample(0, 800, Direction::Right));
  EXPECT_FALSE(sample(1, 800, Direction::Right));
  EXPECT_FALSE(sample(0, 512, Direction::Left));
  EXPECT_FALSE(sample(-1, 512, Direction::Left));
}

TEST_F(CarriageDetectionTest, ComputesSelectionOffsetsForEachCarriage) {
  struct CarriageCase {
    CarriageType type;
    std::initializer_list<uint16_t> sensorValues;
    int16_t leftOffset;
    int16_t rightOffset;
    uint8_t extension;
    uint8_t flags;
  };

  const CarriageCase cases[] = {
      {CarriageType::Knit, {800, 800, 512, 512}, 16, -16, 12, 0},
      {CarriageType::Lace, {100, 100, 512, 512}, 4, -4, 12, 0},
      {CarriageType::Garter, {100, 800, 512, 512}, -8, 8, 18, 0},
      {CarriageType::Knit270,
       {800, 800, 512, 512},
       9,
       -9,
       8,
       HALLSENSOR_K270_K},
  };

  const Direction directions[] = {Direction::Left, Direction::Right};
  for (const CarriageCase &carriageCase : cases) {
    for (Direction direction : directions) {
      config.flags = carriageCase.flags;
      sensor->config(&config);

      Carriage carriage;
      crossCarriage(carriage, carriageCase.type, carriageCase.sensorValues,
                    direction);

      EXPECT_EQ(carriage.getSelectPosition(Direction::Left),
                carriage.getPosition() + carriageCase.leftOffset);
      EXPECT_EQ(carriage.getSelectPosition(Direction::Right),
                carriage.getPosition() + carriageCase.rightOffset);
      EXPECT_EQ(carriage.getExtension(), carriageCase.extension);
    }
  }
}

TEST_F(CarriageDetectionTest, FinishesAfterCarriageSpecificOverrun) {
  Carriage carriage;
  crossCarriage(carriage, CarriageType::Garter, {100, 800, 512, 512});

  EXPECT_FALSE(carriage.workFinished(MachineSide::Left, Direction::Right));
  EXPECT_FALSE(carriage.workFinished(MachineSide::None, Direction::Right));

  for (uint8_t step = 1; step < carriage.getExtension(); ++step) {
    EXPECT_FALSE(carriage.workFinished(MachineSide::Right, Direction::Right));
  }
  EXPECT_TRUE(carriage.workFinished(MachineSide::Right, Direction::Right));
}

TEST(HallSensorConfigTest, RestoresAnalogPinModesAfterKh910Session) {
  NiceMock<Hal_mock> hal;
  ON_CALL(hal, digitalRead(kSensorPin2)).WillByDefault(Return(LOW));

  HallSensor sensor(&hal, kSensorPin, kSensorPin2, kDetectPin);
  HallSensor::Config digital = {
      .position = 199,
      .thresholdLow = 200,
      .thresholdHigh = 600,
      .flags = HALLSENSOR_DIGITAL,
  };
  HallSensor::Config analog = {
      .position = 199,
      .thresholdLow = 200,
      .thresholdHigh = 600,
      .flags = 0,
  };

  EXPECT_CALL(hal, pinMode(kSensorPin, INPUT_PULLUP));
  EXPECT_CALL(hal, pinMode(kSensorPin2, INPUT_PULLUP));
  sensor.config(&digital);

  EXPECT_CALL(hal, pinMode(kSensorPin, INPUT));
  EXPECT_CALL(hal, pinMode(kSensorPin2, INPUT));
  sensor.config(&analog);
}

}  // namespace
