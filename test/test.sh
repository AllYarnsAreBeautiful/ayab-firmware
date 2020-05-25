cmake -V -B ./build -S .
cmake -V --build ./build
cd ./build
GTEST_COLOR=1 ctest -V --output-on-failure .
