#include "shield.h"

namespace Shield {

    // GPIO expander 
    namespace GpioExpanders {
    // I2C addresses definition
        const uint8_t I2C_ADDRESSES[][2] = {
            {0x20, 0x21}, // MCP23008 and PCF8574
            {0x38, 0x39}, // PCF8574A
            {0, 0}        // End of list
        };
    } // namespace GpioExpanders

} // namespace Shield