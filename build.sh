#!/bin/bash

SUFFIX=$1

rm -rf build

mkdir -p build/uno
mkdir -p build/mega2560

make clean
make BOARD=uno MACHINETYPE=KH910
mv ayab.hex build/uno/kh910_uno${SUFFIX}.hex
make clean
make BOARD=uno MACHINETYPE=KH930
mv ayab.hex build/uno/kh930_uno${SUFFIX}.hex
make clean
make BOARD=mega MACHINETYPE=KH910
mv ayab.hex build/mega2560/kh910_mega${SUFFIX}.hex
make clean
make BOARD=mega MACHINETYPE=KH930
mv ayab.hex build/mega2560/kh930_mega${SUFFIX}.hex