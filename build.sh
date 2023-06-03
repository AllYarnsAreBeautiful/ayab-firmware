#!/bin/bash

set -e

OPTIND=1

verbose=0

# parse arguments
shift $((OPTIND-1))
[ "${1:-}" = "--" ] && shift

SUFFIX=$1

# find the number of processors
if command -v nproc &> /dev/null
then
  # linux
  numproc=$(nproc)
else
  # mac
  numproc=$(sysctl -n hw.logicalcpu)
fi

parent_path=$( cd "$(dirname "${BASH_SOURCE[0]}")" ; pwd -P )
cd $parent_path

rm -rf build/*
mkdir -p build

# change to src directory
cd "$parent_path"/src/ayab

if [ -z ${ARDMK_DIR+x} ]; then
  export ARDMK_DIR=$(dirname $(find /usr -name Arduino.mk))
fi

function make_variant() {
  subboard=""
  make BOARD_TAG=$1 $subboard MACHINETYPE=$2 -j $numproc
  mv $parent_path/build/raw/$1/$2/ayab.hex $parent_path/build/ayab_$2_$1.hex
}

function make_board() {
  make_variant $1 monolithic
}

make_board uno

cd $parent_path
rm -rf build/raw
