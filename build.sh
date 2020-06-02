#!/bin/bash

set -e

OPTIND=1

verbose=0
hw_tests=0

# parse arguments
while getopts "v" opt; do
    case $opt in
        v)   verbose=1
             ;;
        hw)   hw_tests=1
             ;;
    esac
done
shift $((OPTIND-1))
[ "${1:-}" = "--" ] && shift

SUFFIX=$1

parent_path=$( cd "$(dirname "${BASH_SOURCE[0]}")" ; pwd -P )
cd $parent_path

rm -rf build/*
mkdir -p build

# change to src directory
cd "$parent_path"/src/ayab

if [ -z ${ARDMK_DIR+x} ]; then
  export ARDMK_DIR=$(dirname $(find /usr -name Arduino.mk))
fi

if [[ $hw_tests -eq 1 ]]; then

  # HW tests
  CPPFLAGS=-DAYAB_HW_TEST make BOARD_TAG=uno MACHINETYPE=KH910 -j $(nproc)
  make BOARD_TAG=uno clean -j $(nproc)
  CPPFLAGS=-DAYAB_HW_TEST make BOARD_TAG=uno MACHINETYPE=KH930 -j $(nproc)
  CPPFLAGS=-DAYAB_HW_TEST make BOARD_TAG=mega BOARD_SUB=atmega2560 MACHINETYPE=KH910 -j $(nproc)
  make BOARD_TAG=mega BOARD_SUB=atmega2560 clean -j $(nproc)
  CPPFLAGS=-DAYAB_HW_TEST make BOARD_TAG=mega BOARD_SUB=atmega2560 MACHINETYPE=KH930 -j $(nproc)

  make BOARD_TAG=uno clean -j $(nproc)
  make BOARD_TAG=mega BOARD_SUB=atmega2560 clean -j $(nproc)

fi

function make_variant() {
  subboard=""
  if [[ $1 == "mega" ]]; then
    subboard="BOARD_SUB=atmega2560"
  fi
    make BOARD_TAG=$1 $subboard MACHINETYPE=$2 -j $(nproc)
    mv $parent_path/build/raw/$1/$2/ayab.hex $parent_path/build/ayab_$2_$1.hex
}

function make_machine() {
  make_variant uno $1
  make_variant mega $1
}

make_machine KH910
make_machine KH930

cd $parent_path
rm -rf build/raw
