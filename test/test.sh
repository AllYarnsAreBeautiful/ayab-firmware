#!/bin/bash
set -e
# change to script directory
parent_path=$( cd "$(dirname "${BASH_SOURCE[0]}")" ; pwd -P )
cd "$parent_path"

cmake -B ./build -S .
cmake --build ./build
cd ./build
GTEST_COLOR=1 ctest --output-on-failure .
cd ../..
gcovr -r . -e test_* -e arduino_mock* --html-details ./test/build/coverage.html --html-title ayab-test
gcovr -r . -e test_* -e arduino_mock*
gcovr -r . --branches -e test_* -e arduino_mock*
