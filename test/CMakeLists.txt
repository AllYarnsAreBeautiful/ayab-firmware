cmake_minimum_required(VERSION 3.1)
project(ayab_test)

set(CMAKE_CXX_STANDARD 11 CACHE STRING "Set the C++ standard to be used for compiling")

find_package(Threads REQUIRED)

add_subdirectory(arduino_mock)

# Includes arduino-mock files directly inplace of original Arduino headers.
set(COMMON_INCLUDES
    ${ARDUINO_MOCK_INCLUDE_DIRS}
    ${ARDUINO_MOCK_LIBS_DIR}/lib/gtest/gtest/src/gtest/googletest/include
    ${ARDUINO_MOCK_LIBS_DIR}/lib/gtest/gtest/src/gtest/googlemock/include
    ${PROJECT_SOURCE_DIR}/mocks
    # External libraries
    ${PROJECT_SOURCE_DIR}/../libraries/Alt_MCP23008
    ${PROJECT_SOURCE_DIR}/../libraries/SoftI2CMaster
    )
set(COMMON_SOURCES
    ${PROJECT_SOURCE_DIR}/./test_all.cpp

    ${PROJECT_SOURCE_DIR}/../encoders.cpp
    ${PROJECT_SOURCE_DIR}/./test_encoders.cpp

    ${PROJECT_SOURCE_DIR}/../beeper.cpp
    ${PROJECT_SOURCE_DIR}/./test_beeper.cpp

    ${PROJECT_SOURCE_DIR}/../solenoids.cpp
    ${PROJECT_SOURCE_DIR}/./test_solenoids.cpp

    ${PROJECT_SOURCE_DIR}/../knitter.cpp
    ${PROJECT_SOURCE_DIR}/./test_knitter.cpp
    )
set(COMMON_DEFINES
    ARDUINO=106
    AYAB_QUIET
    AYAB_TESTS
    )
set(COMMON_FLAGS
    -Wall -Wextra -Wpedantic
    -fprofile-arcs -ftest-coverage
    -g -Og
    )
set(COMMON_LINKER_FLAGS
    ${ARDUINO_MOCK_LIBS_DIR}/lib/gtest/gtest/src/gtest-build/googlemock/gtest/libgtest.a
    ${ARDUINO_MOCK_LIBS_DIR}/lib/gtest/gtest/src/gtest-build/googlemock/libgmock.a
    ${ARDUINO_MOCK_LIBS_DIR}/dist/lib/libarduino_mock.a
    ${CMAKE_THREAD_LIBS_INIT}
    -lgcov
    )

# KH930 Uno
add_executable(${PROJECT_NAME}_KH930_Uno
    ${COMMON_SOURCES}
    # External libraries
    ${PROJECT_SOURCE_DIR}/../libraries/Alt_MCP23008/Alt_MCP23008.cpp
    )
target_include_directories(${PROJECT_NAME}_KH930_Uno
    PRIVATE
    ${COMMON_INCLUDES}
    )
target_compile_definitions(${PROJECT_NAME}_KH930_Uno
    PRIVATE
    ${COMMON_DEFINES}
    KH930
    __AVR_ATmega168__
    )
target_compile_options(${PROJECT_NAME}_KH930_Uno PRIVATE
    ${COMMON_FLAGS}
    )
target_link_libraries(${PROJECT_NAME}_KH930_Uno
    ${COMMON_LINKER_FLAGS}
    )
add_dependencies(${PROJECT_NAME}_KH930_Uno arduino_mock)

# KH910 Uno
add_executable(${PROJECT_NAME}_KH910_Uno
    ${COMMON_SOURCES}
    # External libraries
    ${PROJECT_SOURCE_DIR}/../libraries/Alt_MCP23008/Alt_MCP23008.cpp
    )
target_include_directories(${PROJECT_NAME}_KH910_Uno
    PRIVATE
    ${COMMON_INCLUDES}
    )
target_compile_definitions(${PROJECT_NAME}_KH910_Uno
    PRIVATE
    ${COMMON_DEFINES}
    KH910
    __AVR_ATmega168__
    )
target_compile_options(${PROJECT_NAME}_KH910_Uno PRIVATE
    ${COMMON_FLAGS}
    )
target_link_libraries(${PROJECT_NAME}_KH910_Uno
    ${COMMON_LINKER_FLAGS}
    )
add_dependencies(${PROJECT_NAME}_KH910_Uno arduino_mock)

# KH930 Mega
add_executable(${PROJECT_NAME}_KH930_Mega
    ${COMMON_SOURCES}
    # External libraries
    ${PROJECT_SOURCE_DIR}/../libraries/SoftI2CMaster/SoftI2CMaster.cpp
    )
target_include_directories(${PROJECT_NAME}_KH930_Mega
    PRIVATE
    ${COMMON_INCLUDES}
    )
target_compile_definitions(${PROJECT_NAME}_KH930_Mega
    PRIVATE
    ${COMMON_DEFINES}
    KH930
    __AVR_ATmega1280__
    )
target_compile_options(${PROJECT_NAME}_KH930_Mega PRIVATE
    ${COMMON_FLAGS}
    )
target_link_libraries(${PROJECT_NAME}_KH930_Mega
    ${COMMON_LINKER_FLAGS}
    )
add_dependencies(${PROJECT_NAME}_KH930_Mega arduino_mock)

# KH910 Mega
add_executable(${PROJECT_NAME}_KH910_Mega
    ${COMMON_SOURCES}
    # External libraries
    ${PROJECT_SOURCE_DIR}/../libraries/SoftI2CMaster/SoftI2CMaster.cpp
    )
target_include_directories(${PROJECT_NAME}_KH910_Mega
    PRIVATE
    ${COMMON_INCLUDES}
    )
target_compile_definitions(${PROJECT_NAME}_KH910_Mega
    PRIVATE
    ${COMMON_DEFINES}
    KH910
    __AVR_ATmega1280__
    )
target_compile_options(${PROJECT_NAME}_KH910_Mega PRIVATE
    ${COMMON_FLAGS}
    )
target_link_libraries(${PROJECT_NAME}_KH910_Mega
    ${COMMON_LINKER_FLAGS}
    )
add_dependencies(${PROJECT_NAME}_KH910_Mega arduino_mock)

enable_testing()
add_test(test_all ${PROJECT_NAME}_KH930_Uno)
add_test(test_all ${PROJECT_NAME}_KH910_Uno)
add_test(test_all ${PROJECT_NAME}_KH930_Mega)
add_test(test_all ${PROJECT_NAME}_KH910_Mega)
