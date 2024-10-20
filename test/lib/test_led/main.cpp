#include <gtest/gtest.h>
#include <list>

#include "shield.h"
#include "led.h"
#include "mocks/hal_mock.h"

using namespace std;

using ::testing::NiceMock;
using ::testing::Return;
using ::testing::ReturnPointee;
using ::testing::_;

void ledExpectToggle(NiceMock<Hal_mock> *_hal_mock, uint8_t pin, uint8_t state) {
    EXPECT_CALL(*_hal_mock, digitalWrite(pin, state)).Times(1);
    EXPECT_CALL(*_hal_mock, millis()).Times(2);
}

void ledExpectNoToggle(NiceMock<Hal_mock> *_hal_mock, uint8_t pin) {
    EXPECT_CALL(*_hal_mock, digitalWrite(pin, _)).Times(0);
    EXPECT_CALL(*_hal_mock, millis()).Times(1);
}

class ledTest : public ::testing::Test {
protected:
    Led* _led{nullptr};
    NiceMock<Hal_mock>* _hal_mock{nullptr};
    unsigned long currentTime = 0;

    virtual void SetUp() {
      _hal_mock = new NiceMock<Hal_mock>(); 

      // millis() callback
      ON_CALL(*_hal_mock, millis()).WillByDefault(ReturnPointee(&currentTime));

      // Check Constructor
      EXPECT_CALL(*_hal_mock, digitalWrite(LED_A_PIN, 0x0)).Times(1);
      EXPECT_CALL(*_hal_mock, millis()).Times(1);
      EXPECT_CALL(*_hal_mock, pinMode(LED_A_PIN, OUTPUT)).Times(1);

      _led = new Led(_hal_mock, LED_A_PIN);
    }

    virtual void TearDown() {
      delete _led;
      delete _hal_mock;
    }
};

TEST_F(ledTest, constructor2) {
   delete _led;

   // Check 2nd Constructor
   EXPECT_CALL(*_hal_mock, digitalWrite(LED_A_PIN, 0x1)).Times(1);
   EXPECT_CALL(*_hal_mock, millis()).Times(1);
   EXPECT_CALL(*_hal_mock, pinMode(LED_A_PIN, OUTPUT)).Times(1);

   _led = new Led(_hal_mock, LED_A_PIN, 0x1, 0x0);
}

TEST_F(ledTest, led_onoff) {
    // Configure on mode and check state change
    currentTime = 1000;
    EXPECT_CALL(*_hal_mock, digitalWrite(LED_A_PIN, 0x1)).Times(1);
    EXPECT_CALL(*_hal_mock, millis()).Times(1);
    _led->on();

    // Check that there are no state changes anymore
    currentTime = 10000;
    ledExpectNoToggle(_hal_mock, LED_A_PIN);
    _led->schedule();

    // Configure off mode and check state change
    currentTime = 20000;
    EXPECT_CALL(*_hal_mock, digitalWrite(LED_A_PIN, 0x0)).Times(1);
    EXPECT_CALL(*_hal_mock, millis()).Times(1);
    _led->off();

    // Check that there are no state changes anymore
    currentTime = 30000;
    ledExpectNoToggle(_hal_mock, LED_A_PIN);
    _led->schedule();
}

TEST_F(ledTest, led_blink) {
    // Configure blink mode and check state change
    currentTime = 1000;
    EXPECT_CALL(*_hal_mock, digitalWrite(LED_A_PIN, 0x1)).Times(1);
    EXPECT_CALL(*_hal_mock, millis()).Times(1);
    _led->blink(100, 1000);

    // Check state change after timeOn
    currentTime = 2000;
    ledExpectToggle(_hal_mock, LED_A_PIN, 0x0);
    _led->schedule();

    // Check state change after timeOff
    currentTime = 2100;
    ledExpectToggle(_hal_mock, LED_A_PIN, 0x1);
    _led->schedule();

    // Check that there are no state changes within timeOn
    list<unsigned long> timeSamples = {2200, 2300, 3099};
    for(auto it = timeSamples.begin();it != timeSamples.end(); it++) {
      currentTime = *it;
      ledExpectNoToggle(_hal_mock, LED_A_PIN);
      _led->schedule();
    }
}

TEST_F(ledTest, led_flash) {
    // Configure flash mode and check state change
    currentTime = 1000;
    EXPECT_CALL(*_hal_mock, digitalWrite(LED_A_PIN, 0x1)).Times(1);
    EXPECT_CALL(*_hal_mock, millis()).Times(1);
    _led->flash(100);

    // Check state change after timeOn
    currentTime = 1100;
    ledExpectToggle(_hal_mock, LED_A_PIN, 0x0);
    _led->schedule();

    // Check that there are no state changes anymore
    list<unsigned long> timeSamples = {1200, 1200, 10000};
    for(auto it = timeSamples.begin();it != timeSamples.end(); it++) {
      currentTime = *it;
      ledExpectNoToggle(_hal_mock, LED_A_PIN);
      _led->schedule();
    }
}