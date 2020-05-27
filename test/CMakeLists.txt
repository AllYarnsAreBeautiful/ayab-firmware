cmake_minimum_required(VERSION 3.1)
project(ayab_test)

set(CMAKE_CXX_STANDARD 11 CACHE STRING "Set the C++ standard to be used for compiling")

find_package(Threads REQUIRED)

add_subdirectory(arduino_mock)

# Includes arduino-mock files directly inplace of original Arduino headers.
include_directories(
    ${ARDUINO_MOCK_INCLUDE_DIRS}/arduino-mock
    ${ARDUINO_MOCK_LIBS_DIR}/lib/gtest/gtest/src/gtest/googletest/include
    ${ARDUINO_MOCK_LIBS_DIR}/lib/gtest/gtest/src/gtest/googlemock/include
    )

add_executable(${PROJECT_NAME}
    ${PROJECT_SOURCE_DIR}/./test_all.cpp

    ${PROJECT_SOURCE_DIR}/../encoders.cpp
    ${PROJECT_SOURCE_DIR}/./test_encoders.cpp

    ${PROJECT_SOURCE_DIR}/../beeper.cpp
    ${PROJECT_SOURCE_DIR}/./test_beeper.cpp
    )

target_compile_definitions(${PROJECT_NAME}
    PRIVATE
    AYAB_QUIET
    KH910
    )
target_compile_options(${PROJECT_NAME} PRIVATE
    -Wall -Wextra -Wpedantic
    -Werror
    -fprofile-arcs -ftest-coverage
    -g -Og
    )

target_link_libraries(${PROJECT_NAME}
    ${ARDUINO_MOCK_LIBS_DIR}/lib/gtest/gtest/src/gtest-build/googlemock/gtest/libgtest.a
    ${ARDUINO_MOCK_LIBS_DIR}/lib/gtest/gtest/src/gtest-build/googlemock/libgmock.a
    ${ARDUINO_MOCK_LIBS_DIR}/dist/lib/libarduino_mock.a
    ${CMAKE_THREAD_LIBS_INIT}
    -lgcov
    )

add_dependencies(${PROJECT_NAME} arduino_mock)

enable_testing()
add_test(test_all ${PROJECT_NAME})
