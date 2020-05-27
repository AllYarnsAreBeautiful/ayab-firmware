#pragma once

// TODO(sl): These should be a part of the Arduino mock
#define _delay_us(x) (void)(x)
#define digitalPinToBitMask(x) (x)
#define digitalPinToPort(x) (x)
#define portOutputRegister(x) (&x)
#define portInputRegister(x) (&x)
#define portModeRegister(x) (&x)
