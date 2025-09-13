#ifndef SHIELD_H
#define SHIELD_H

#include <stdint.h>

namespace Shield {

    // LEDs
    struct Leds {
        static constexpr uint8_t LED_A_PIN = 5;  // green
        static constexpr uint8_t LED_B_PIN = 6;  // yellow
    };

    // Piezo
    struct Piezo {
        static constexpr uint8_t PIEZO_PIN = 9;
    };

    // Encoder
    struct Encoder {
        static constexpr uint8_t ENC_A_PIN = 2;  // V1
        static constexpr uint8_t ENC_B_PIN = 3;  // V2
        static constexpr uint8_t ENC_C_PIN = 4;  // BP
    };

    // Hall detectors
    struct HallDetectors {
        #if defined(ARDUINO_AVR_UNO) || defined(ARDUINO_UNOR4_WIFI) || defined(ARDUINO_UNOR4_MINIMA)
            static constexpr uint8_t EOL_R_PIN = 14; // AN0/D14: Right (K digital input for KH910)
            static constexpr uint8_t EOL_L_PIN = 15; // AN1/D15: Left
        #elif defined(ARDUINO_AVR_MEGA2560)
            static constexpr uint8_t EOL_R_PIN = 54; // AN0/D54: Right (K digital input for KH910)
            static constexpr uint8_t EOL_L_PIN = 55; // AN1/D55: Left
        #endif
        static constexpr uint8_t EOL_R_L_PIN = 7; // Right L digital input for KH910
        static constexpr uint8_t EOL_R_DETECT_PIN = 8; // Shorted to EOL_R_L_PIN to detect HW fix
    };

    // GPIO expanders
    namespace GpioExpanders {
        // I2C addresses declaration, ends with a sentinel row {0, 0}.
        extern const uint8_t I2C_ADDRESSES[][2];
    };

};
#endif // SHIELD_H
