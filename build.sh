#!/bin/bash

set -e

OPTIND=1

verbose=0
hw_tests=0

# parse arguments
while getopts "w" opt; do
    case $opt in
        w)   hw_tests=1
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

function make_hw_test() {
  subboard=""
  if [[ $1 == "mega" ]]; then
    subboard="BOARD_SUB=atmega2560"
  fi
  make BOARD_TAG=$1 $subboard MACHINETYPE=HW_TEST -j $(nproc)
  mv $parent_path/build/raw/$1/HW_TEST/ayab.hex $parent_path/build/ayab_HW_TEST_$1.hex
}

function make_variant() {
  subboard=""
  if [[ $1 == "mega" ]]; then
    subboard="BOARD_SUB=atmega2560"
  fi
    make BOARD_TAG=$1 $subboard MACHINETYPE=$2 -j $(nproc)
    mv $parent_path/build/raw/$1/$2/ayab.hex $parent_path/build/ayab_$2_$1.hex
}

function make_board() {
  make_variant $1 KH910
  make_variant $1 KH930
if [[ $hw_tests -eq 1 ]]; then
  echo "Making HW Test $1"
  make_hw_test $1
fi
}

make_board uno
make_board mega

cd $parent_path
rm -rf build/raw
