cmake_minimum_required(VERSION 2.8.8)
project(ayab_test)

set(CMAKE_CXX_STANDARD 11 CACHE STRING "Set the C++ standard to be used for compiling")

message("building all tests")

find_package(Threads REQUIRED)

add_subdirectory(arduino_mock)

# Includes arduino-mock files directly inplace of original Arduino headers.
include_directories(
    ${ARDUINO_MOCK_INCLUDE_DIRS}/arduino-mock
    ${ARDUINO_MOCK_LIBS_DIR}/lib/gtest/gtest/src/gtest/googletest/include
    ${ARDUINO_MOCK_LIBS_DIR}/lib/gtest/gtest/src/gtest/googlemock/include
)

set(TEST_SRCS
  ./test_all.cpp
  ./test_encoders.cpp
)

set(PROGRAM_SRCS
  ../encoders.cpp
)

add_executable(${PROJECT_NAME} ${TEST_SRCS} ${PROGRAM_SRCS})

target_compile_definitions(${PROJECT_NAME} PRIVATE KH910)

target_link_libraries(${PROJECT_NAME}
    ${ARDUINO_MOCK_LIBS_DIR}/lib/gtest/gtest/src/gtest-build/googlemock/gtest/libgtest.a
    ${ARDUINO_MOCK_LIBS_DIR}/lib/gtest/gtest/src/gtest-build/googlemock/libgmock.a
    ${ARDUINO_MOCK_LIBS_DIR}/dist/lib/libarduino_mock.a
    ${CMAKE_THREAD_LIBS_INIT}
)

add_dependencies(${PROJECT_NAME} arduino_mock)

enable_testing()
add_test(test_all ${PROJECT_NAME})
