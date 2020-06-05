#!/bin/bash
clang-tidy -header-filter='.*' -checks='*,-llvm-header-guard,-fuchsia-trailing-return' src/ayab/$1.cpp --fix -- -isystem /usr/share/arduino/hardware/arduino/cores/arduino/ -isystem /usr/share/arduino/hardware/tools/avr/lib/avr/include/ -isystem /usr/share/arduino/hardware/arduino/variants/standard -isystem libraries/SerialCommand/ -isystem libraries/Adafruit_MCP23008/ "${@:2}"
