cmake_minimum_required(VERSION 3.1)
project(ayab_test)

set(CMAKE_CXX_STANDARD 11 CACHE STRING "Set the C++ standard to be used for compiling")

find_package(Threads REQUIRED)

add_subdirectory(arduino_mock)

set(SOURCE_DIRECTORY
    ${PROJECT_SOURCE_DIR}/../src/ayab
    )
set(LIBRARY_DIRECTORY
    ${PROJECT_SOURCE_DIR}/../libraries
    )

# Includes arduino-mock files directly inplace of original Arduino headers.
set(COMMON_INCLUDES
    ${ARDUINO_MOCK_INCLUDE_DIRS}
    ${ARDUINO_MOCK_LIBS_DIR}/lib/gtest/gtest/src/gtest/googletest/include
    ${ARDUINO_MOCK_LIBS_DIR}/lib/gtest/gtest/src/gtest/googlemock/include
    ${PROJECT_SOURCE_DIR}/mocks
    ${SOURCE_DIRECTORY}
    )
set(EXTERNAL_LIB_INCLUDES
    ${LIBRARY_DIRECTORY}/Adafruit_MCP23008
    ${LIBRARY_DIRECTORY}/SoftI2CMaster
    )
set(COMMON_SOURCES
    ${PROJECT_SOURCE_DIR}/test_all.cpp

    ${SOURCE_DIRECTORY}/encoders.cpp
    ${PROJECT_SOURCE_DIR}/test_encoders.cpp

    ${SOURCE_DIRECTORY}/beeper.cpp
    ${PROJECT_SOURCE_DIR}/test_beeper.cpp

    ${SOURCE_DIRECTORY}/solenoids.cpp
    ${PROJECT_SOURCE_DIR}/test_solenoids.cpp

    ${SOURCE_DIRECTORY}/serial_encoding.cpp
    ${PROJECT_SOURCE_DIR}/test_serial_encoding.cpp
    ${PROJECT_SOURCE_DIR}/mocks/serial_encoding/knitter_mock.cpp
    )
set(COMMON_DEFINES
    ARDUINO=105
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
    # For some reason the order of libarduinomock and libgmock matter, keep them this way.
    ${ARDUINO_MOCK_LIBS_DIR}/dist/lib/libarduino_mock.a
    ${ARDUINO_MOCK_LIBS_DIR}/lib/gtest/gtest/src/gtest-build/googlemock/libgmock.a
    ${CMAKE_THREAD_LIBS_INIT}
    -lgcov
    )
set(HARD_I2C_LIB
    ${LIBRARY_DIRECTORY}/Adafruit_MCP23008/Adafruit_MCP23008.cpp
    )
set(SOFT_I2C_LIB
    ${LIBRARY_DIRECTORY}/SoftI2CMaster/SoftI2CMaster.cpp
    )
function(add_machine machine board)
    if(${board}=="Uno")
        set(processor
            __AVR_ATmega168__
            )
        set(I2C_LIB
            ${HARD_I2C_LIB}
            )
    else()
        set(processor
            __AVR_ATmega2560__
            )
        set(I2C_LIB
            ${SOFT_I2C_LIB}
            )
    endif()
    add_executable(${PROJECT_NAME}_${machine}_${board}
        ${COMMON_SOURCES}
        # External libraries
        ${I2C_LIB}
        )
    target_include_directories(${PROJECT_NAME}_${machine}_${board}
        PRIVATE
        ${COMMON_INCLUDES}
        ${EXTERNAL_LIB_INCLUDES}
        )
    target_compile_definitions(${PROJECT_NAME}_${machine}_${board}
        PRIVATE
        ${COMMON_DEFINES}
        ${machine}
        ${processor}
        )
    target_compile_options(${PROJECT_NAME}_${machine}_${board} PRIVATE
        ${COMMON_FLAGS}
        )
    target_link_libraries(${PROJECT_NAME}_${machine}_${board}
        ${COMMON_LINKER_FLAGS}
        )
endfunction()

add_machine(KH930 Uno)
add_machine(KH910 Uno)
add_machine(KH930 Mega)
add_machine(KH910 Mega)

# Knitter (device independent)
add_executable(${PROJECT_NAME}_knitter
    ${PROJECT_SOURCE_DIR}/test_all.cpp
    ${SOURCE_DIRECTORY}/knitter.cpp
    ${PROJECT_SOURCE_DIR}/test_knitter.cpp
    ${PROJECT_SOURCE_DIR}/mocks/knitter/solenoids_mock.cpp
    ${PROJECT_SOURCE_DIR}/mocks/knitter/encoders_mock.cpp
    ${PROJECT_SOURCE_DIR}/mocks/knitter/beeper_mock.cpp
)
target_include_directories(${PROJECT_NAME}_knitter
    PRIVATE
    ${COMMON_INCLUDES}
)
target_compile_definitions(${PROJECT_NAME}_knitter
    PRIVATE
    ${COMMON_DEFINES}
    KH910 # should not matter which one is chosen for knitter
    __AVR_ATmega1280__ # should not matter which one is chosen for knitter
)
target_compile_options(${PROJECT_NAME}_knitter PRIVATE
    ${COMMON_FLAGS}
)
target_link_libraries(${PROJECT_NAME}_knitter
    ${COMMON_LINKER_FLAGS}
)
add_dependencies(${PROJECT_NAME}_knitter arduino_mock)

enable_testing()
add_test(test_KH930_Uno ${PROJECT_NAME}_KH930_Uno)
add_test(test_KH910_Uno ${PROJECT_NAME}_KH910_Uno)
add_test(test_KH930_Mega ${PROJECT_NAME}_KH930_Mega)
add_test(test_KH910_Mega ${PROJECT_NAME}_KH910_Mega)
add_test(test_knitter ${PROJECT_NAME}_knitter)
