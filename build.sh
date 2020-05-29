#!/bin/bash
set -e

SUFFIX=$1

rm -rf build

mkdir -p build/uno
mkdir -p build/mega2560

# Uno
make BOARD_TAG=uno MACHINETYPE=KH910 clean
make BOARD_TAG=uno MACHINETYPE=KH910
mv build-uno/ayab.hex build/uno/kh910_uno${SUFFIX}.hex

make BOARD_TAG=uno MACHINETYPE=KH930 clean
make BOARD_TAG=uno MACHINETYPE=KH930
mv build-uno/ayab.hex build/uno/kh930_uno${SUFFIX}.hex

make BOARD_TAG=uno clean

# Mega
make BOARD_TAG=mega BOARD_SUB=atmega2560 MACHINETYPE=KH910 clean
make BOARD_TAG=mega BOARD_SUB=atmega2560 MACHINETYPE=KH910
mv build-mega-atmega2560/ayab.hex build/mega2560/kh910_mega${SUFFIX}.hex

make BOARD_TAG=mega BOARD_SUB=atmega2560 MACHINETYPE=KH930 clean
make BOARD_TAG=mega BOARD_SUB=atmega2560 MACHINETYPE=KH930
mv build-mega-atmega2560/ayab.hex build/mega2560/kh930_mega${SUFFIX}.hex

make BOARD_TAG=mega BOARD_SUB=atmega2560 clean

# HW tests
CPPFLAGS=-DAYAB_HW_TEST make BOARD_TAG=uno MACHINETYPE=KH910
CPPFLAGS=-DAYAB_HW_TEST make BOARD_TAG=uno MACHINETYPE=KH930
CPPFLAGS=-DAYAB_HW_TEST make BOARD_TAG=mega BOARD_SUB=atmega2560 MACHINETYPE=KH910
CPPFLAGS=-DAYAB_HW_TEST make BOARD_TAG=mega BOARD_SUB=atmega2560 MACHINETYPE=KH930
