#!/bin/bash
clang-tidy src/ayab/$1.cpp --fix -- -isystem /usr/share/arduino/hardware/arduino/cores/arduino/ -isystem /usr/lib/avr/include/ -isystem /usr/share/arduino/hardware/arduino/variants/standard -isystem /usr/share/arduino/libraries/Wire/ -DCLANG_TIDY "${@:2}"
