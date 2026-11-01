#include <gtest/gtest.h>

#include <stdint.h>
#include <string.h>

#include "line.h"

namespace {

constexpr uint8_t kKh270PatternBytes = 14;
constexpr uint8_t kStandardPatternBytes = 25;

class LineTest : public ::testing::Test {
 protected:
  void SetUp() override { line.reset(); }

  Line line;
};

TEST_F(LineTest, ResetStartsAtLineZeroInFinishedState) {
  EXPECT_EQ(line.getNextLineNumber(), 0);
  EXPECT_FALSE(line.isLastLine());
  EXPECT_FALSE(line.requested);
  EXPECT_TRUE(line.finished);
}

TEST_F(LineTest, StoresStandardGaugePattern) {
  uint8_t pattern[kStandardPatternBytes];
  memset(pattern, 0xff, sizeof(pattern));
  pattern[0] = 0xfc;
  pattern[24] = 0x7f;

  ASSERT_TRUE(line.setPattern(0, pattern, sizeof(pattern), true));

  EXPECT_TRUE(line.getNeedleValue(0));
  EXPECT_TRUE(line.getNeedleValue(1));
  EXPECT_FALSE(line.getNeedleValue(2));
  EXPECT_TRUE(line.getNeedleValue(199));
  EXPECT_TRUE(line.isLastLine());
  EXPECT_FALSE(line.finished);
  EXPECT_EQ(line.getNextLineNumber(), 1);
}

TEST_F(LineTest, StoresKh270PatternWithoutReadingPastPayload) {
  uint8_t previousPattern[kStandardPatternBytes] = {};
  uint8_t pattern[kKh270PatternBytes];
  memset(pattern, 0xff, sizeof(pattern));
  pattern[0] = 0xfe;
  pattern[13] = 0x7f;

  ASSERT_TRUE(
      line.setPattern(0, previousPattern, sizeof(previousPattern), false));
  ASSERT_TRUE(line.setPattern(1, pattern, sizeof(pattern), false));

  EXPECT_TRUE(line.getNeedleValue(0));
  EXPECT_FALSE(line.getNeedleValue(1));
  EXPECT_TRUE(line.getNeedleValue(111));
  // Bytes outside the KH270 payload retain the reset value.
  EXPECT_FALSE(line.getNeedleValue(112));
  EXPECT_FALSE(line.getNeedleValue(199));
}

TEST_F(LineTest, RejectsInvalidPatternWithoutAdvancingLine) {
  uint8_t pattern[kStandardPatternBytes] = {};

  EXPECT_FALSE(line.setPattern(1, pattern, sizeof(pattern), false));
  EXPECT_FALSE(line.setPattern(0, nullptr, sizeof(pattern), false));
  EXPECT_FALSE(line.setPattern(0, pattern, 0, false));
  EXPECT_FALSE(line.setPattern(0, pattern, sizeof(pattern) + 1, false));

  EXPECT_EQ(line.getNextLineNumber(), 0);
  EXPECT_TRUE(line.finished);
}

TEST_F(LineTest, ReturnsResetStateForOutOfRangeNeedles) {
  uint8_t pattern[kStandardPatternBytes] = {};
  ASSERT_TRUE(line.setPattern(0, pattern, sizeof(pattern), false));

  EXPECT_TRUE(line.getNeedleValue(-1));
  EXPECT_TRUE(line.getNeedleValue(200));
}

}  // namespace
