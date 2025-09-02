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
        // Analog
        struct Analog {
            static constexpr uint8_t EOL_R_PIN = 0;  // Right
            static constexpr uint8_t EOL_L_PIN = 1;  // Left
        };
        // Digital (KH910 RHS)
        struct Digital {
            static constexpr uint8_t EOL_R_L_PIN = 7;
            static constexpr uint8_t EOL_R_DETECT_PIN = 8;
        };
    };

    // GPIO expanders
    namespace GpioExpanders {
        // I2C addresses declaration, ends with a sentinel row {0, 0}.
        extern const uint8_t I2C_ADDRESSES[][2];
    };

};
#endif // SHIELD_H
