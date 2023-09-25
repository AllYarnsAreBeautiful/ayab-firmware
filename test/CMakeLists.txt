cmake_minimum_required(VERSION 3.18.0)
project(ayab_test)

set(CMAKE_CXX_STANDARD 11 CACHE STRING "Set the C++ standard to be used for compiling")

if(false)
    set(CMAKE_CXX_CLANG_TIDY
        clang-tidy
        -header-filter=.
        -checks=modernize-*,bugprone-*,cppcoreguidelines-*
    )
endif()

find_package(Threads REQUIRED)

add_subdirectory(arduino_mock)

set(SOURCE_DIRECTORY
    ${PROJECT_SOURCE_DIR}/../src/ayab
    )
set(LIBRARY_DIRECTORY
    ${PROJECT_SOURCE_DIR}/../lib
    )

# Includes arduino-mock files directly inplace of original Arduino headers.
set(COMMON_INCLUDES
    ${ARDUINO_MOCK_INCLUDE_DIRS}
    ${ARDUINO_MOCK_LIBS_DIR}/lib/gtest/gtest/src/gtest/googletest/include
    ${ARDUINO_MOCK_LIBS_DIR}/lib/gtest/gtest/src/gtest/googlemock/include
    ${PROJECT_SOURCE_DIR}/mocks
    ${SOURCE_DIRECTORY}
    ${LIBRARY_DIRECTORY}/PacketSerial/src
    )
set(EXTERNAL_LIB_INCLUDES
    ${LIBRARY_DIRECTORY}/Adafruit_MCP23008
    #${LIBRARY_DIRECTORY}/avr-libc/include
    )
set(COMMON_SOURCES
    ${PROJECT_SOURCE_DIR}/test_boards.cpp

    ${SOURCE_DIRECTORY}/encoders.cpp
    ${SOURCE_DIRECTORY}/global_encoders.cpp
    ${PROJECT_SOURCE_DIR}/test_encoders.cpp

    ${SOURCE_DIRECTORY}/solenoids.cpp
    ${SOURCE_DIRECTORY}/global_solenoids.cpp
    ${PROJECT_SOURCE_DIR}/test_solenoids.cpp

    ${SOURCE_DIRECTORY}/beeper.cpp
    ${SOURCE_DIRECTORY}/global_beeper.cpp
    ${PROJECT_SOURCE_DIR}/test_beeper.cpp

    ${SOURCE_DIRECTORY}/com.cpp
    ${SOURCE_DIRECTORY}/global_com.cpp
    ${PROJECT_SOURCE_DIR}/test_com.cpp

    ${SOURCE_DIRECTORY}/opIdle.cpp
    ${SOURCE_DIRECTORY}/global_OpIdle.cpp
    ${PROJECT_SOURCE_DIR}/test_OpIdle.cpp

    ${SOURCE_DIRECTORY}/opInit.cpp
    ${SOURCE_DIRECTORY}/global_OpInit.cpp
    ${PROJECT_SOURCE_DIR}/test_OpInit.cpp

    ${SOURCE_DIRECTORY}/opReady.cpp
    ${SOURCE_DIRECTORY}/global_OpReady.cpp
    ${PROJECT_SOURCE_DIR}/test_OpReady.cpp

    ${SOURCE_DIRECTORY}/opTest.cpp
    ${SOURCE_DIRECTORY}/global_OpTest.cpp
    ${PROJECT_SOURCE_DIR}/test_OpTest.cpp

    ${SOURCE_DIRECTORY}/opError.cpp
    ${SOURCE_DIRECTORY}/global_OpError.cpp
    ${PROJECT_SOURCE_DIR}/test_OpError.cpp

    ${SOURCE_DIRECTORY}/global_OpKnit.cpp
    ${PROJECT_SOURCE_DIR}/mocks/opKnit_mock.cpp

    ${SOURCE_DIRECTORY}/global_controller.cpp
    ${PROJECT_SOURCE_DIR}/mocks/controller_mock.cpp
    )
set(COMMON_DEFINES
    ARDUINO=1819
    AYAB_TESTS
    )
set(COMMON_FLAGS
    -Wall
    -Wextra
    -Wpedantic
    -Wno-vla
    -Werror
    -Wno-ignored-qualifiers
    --coverage
    -g -Og
    )
set(COMMON_LINKER_FLAGS
    ${ARDUINO_MOCK_LIBS_DIR}/lib/gtest/gtest/src/gtest-build/lib/libgtest.a
    # For some reason the order of libarduinomock and libgmock matter, keep them this way.
    ${ARDUINO_MOCK_LIBS_DIR}/dist/lib/libarduino_mock.a
    ${ARDUINO_MOCK_LIBS_DIR}/lib/gtest/gtest/src/gtest-build/lib/libgmock.a
    ${CMAKE_THREAD_LIBS_INIT}
    -lgcov
    )
set(HARD_I2C_LIB
    ${LIBRARY_DIRECTORY}/Adafruit_MCP23008/Adafruit_MCP23008.cpp
    )
function(add_board board)
    set(processor # uno
        __AVR_ATmega168__
        )
    set(I2C_LIB
        ${HARD_I2C_LIB}
        )
    add_executable(${PROJECT_NAME}_${board}
        ${COMMON_SOURCES}
        # External libraries
        ${I2C_LIB}
        )
    target_include_directories(${PROJECT_NAME}_${board}
        PRIVATE
        ${COMMON_INCLUDES}
        ${EXTERNAL_LIB_INCLUDES}
        )
    target_compile_definitions(${PROJECT_NAME}_${board}
        PRIVATE
        ${COMMON_DEFINES}
        ${processor}
        )
    target_compile_options(${PROJECT_NAME}_${board}
        PRIVATE
        ${COMMON_FLAGS}
        )
    target_link_libraries(${PROJECT_NAME}_${board}
        ${COMMON_LINKER_FLAGS}
        )
    add_dependencies(${PROJECT_NAME}_${board} arduino_mock)
endfunction()

add_board(uno)

add_executable(${PROJECT_NAME}_knit
    ${PROJECT_SOURCE_DIR}/test_all.cpp

    ${SOURCE_DIRECTORY}/global_beeper.cpp
    ${PROJECT_SOURCE_DIR}/mocks/beeper_mock.cpp

    ${SOURCE_DIRECTORY}/global_com.cpp
    ${PROJECT_SOURCE_DIR}/mocks/com_mock.cpp

    ${SOURCE_DIRECTORY}/global_encoders.cpp
    ${PROJECT_SOURCE_DIR}/mocks/encoders_mock.cpp

    ${SOURCE_DIRECTORY}/global_solenoids.cpp
    ${PROJECT_SOURCE_DIR}/mocks/solenoids_mock.cpp

    ${SOURCE_DIRECTORY}/global_OpIdle.cpp
    ${PROJECT_SOURCE_DIR}/mocks/opIdle_mock.cpp

    ${SOURCE_DIRECTORY}/global_OpInit.cpp
    ${PROJECT_SOURCE_DIR}/mocks/opInit_mock.cpp

    ${SOURCE_DIRECTORY}/global_OpReady.cpp
    ${PROJECT_SOURCE_DIR}/mocks/opReady_mock.cpp

    ${SOURCE_DIRECTORY}/global_OpTest.cpp
    ${PROJECT_SOURCE_DIR}/mocks/opTest_mock.cpp

    ${SOURCE_DIRECTORY}/global_OpError.cpp
    ${PROJECT_SOURCE_DIR}/mocks/opError_mock.cpp

    ${SOURCE_DIRECTORY}/controller.cpp
    ${SOURCE_DIRECTORY}/global_controller.cpp
    ${PROJECT_SOURCE_DIR}/test_controller.cpp

    ${SOURCE_DIRECTORY}/opKnit.cpp
    ${SOURCE_DIRECTORY}/global_OpKnit.cpp
    ${PROJECT_SOURCE_DIR}/test_OpKnit.cpp
)
target_include_directories(${PROJECT_NAME}_knit
    PRIVATE
    ${COMMON_INCLUDES}
    ${EXTERNAL_LIB_INCLUDES}
)
target_compile_definitions(${PROJECT_NAME}_knit
    PRIVATE
    ${COMMON_DEFINES}
    __AVR_ATmega168__
)
target_compile_options(${PROJECT_NAME}_knit PRIVATE
    ${COMMON_FLAGS}
)
target_link_libraries(${PROJECT_NAME}_knit
    ${COMMON_LINKER_FLAGS}
)
add_dependencies(${PROJECT_NAME}_knit arduino_mock)

enable_testing()
include(GoogleTest)
gtest_discover_tests(${PROJECT_NAME}_uno TEST_PREFIX uno_ XML_OUTPUT_DIR ./xml_out)
gtest_discover_tests(${PROJECT_NAME}_knit TEST_PREFIX knit_ XML_OUTPUT_DIR ./xml_out)
