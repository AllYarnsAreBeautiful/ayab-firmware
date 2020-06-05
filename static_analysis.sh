#!/bin/bash
clang-tidy src/ayab/$1.cpp -- -isystem /usr/share/arduino/hardware/arduino/cores/arduino/ -isystem /usr/share/arduino/hardware/tools/avr/lib/avr/include/ -isystem /usr/share/arduino/hardware/arduino/variants/standard -isystem libraries/SerialCommand/ -isystem libraries/Adafruit_MCP23008/ -isystem /usr/share/arduino/libraries/Wire/ -isystem libraries/SoftI2CMaster/ "${@:2}"
