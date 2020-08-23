cmake_minimum_required(VERSION 3.7.2)
project(ayab_test)

set(CMAKE_CXX_STANDARD 11 CACHE STRING "Set the C++ standard to be used for compiling")

# TODO(sl): Create a clang-tidy option, this is really slow!
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
    ${PROJECT_SOURCE_DIR}/../libraries
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
    ${LIBRARY_DIRECTORY}/SoftI2CMaster
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

    ${SOURCE_DIRECTORY}/tester.cpp
    ${SOURCE_DIRECTORY}/global_tester.cpp
    ${PROJECT_SOURCE_DIR}/test_tester.cpp

    ${SOURCE_DIRECTORY}/global_knitter.cpp
    ${PROJECT_SOURCE_DIR}/mocks/knitter_mock.cpp

    ${SOURCE_DIRECTORY}/global_fsm.cpp
    ${PROJECT_SOURCE_DIR}/mocks/fsm_mock.cpp
    )
set(COMMON_DEFINES
    ARDUINO=105
    AYAB_QUIET
    AYAB_TESTS
    AYAB_ENABLE_CRC
    )
set(COMMON_FLAGS
    -Wall -Wextra -Wpedantic
    -Wno-vla
    -Werror
    --coverage
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
function(add_board board)
    if(${board}=="Uno")
        set(processor
            # FIXME(TP): should this be ATmega328?
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
    target_compile_options(${PROJECT_NAME}_${board} PRIVATE
        ${COMMON_FLAGS}
        )
    target_link_libraries(${PROJECT_NAME}_${board}
        ${COMMON_LINKER_FLAGS}
        )
    add_dependencies(${PROJECT_NAME}_${board} arduino_mock)
endfunction()

add_board(Uno)
add_board(Mega)

# Knitter (device independent)
add_executable(${PROJECT_NAME}_knitter
    ${PROJECT_SOURCE_DIR}/test_all.cpp

    ${SOURCE_DIRECTORY}/global_beeper.cpp
    ${PROJECT_SOURCE_DIR}/mocks/beeper_mock.cpp

    ${SOURCE_DIRECTORY}/global_com.cpp
    ${PROJECT_SOURCE_DIR}/mocks/com_mock.cpp

    ${SOURCE_DIRECTORY}/global_encoders.cpp
    ${PROJECT_SOURCE_DIR}/mocks/encoders_mock.cpp

    ${SOURCE_DIRECTORY}/global_solenoids.cpp
    ${PROJECT_SOURCE_DIR}/mocks/solenoids_mock.cpp

    ${SOURCE_DIRECTORY}/global_tester.cpp
    ${PROJECT_SOURCE_DIR}/mocks/tester_mock.cpp

    ${SOURCE_DIRECTORY}/fsm.cpp
    ${SOURCE_DIRECTORY}/global_fsm.cpp
    ${PROJECT_SOURCE_DIR}/test_fsm.cpp

    ${SOURCE_DIRECTORY}/knitter.cpp
    ${SOURCE_DIRECTORY}/global_knitter.cpp
    ${PROJECT_SOURCE_DIR}/test_knitter.cpp

    ${SOFT_I2C_LIB}
)
target_include_directories(${PROJECT_NAME}_knitter
    PRIVATE
    ${COMMON_INCLUDES}
    ${EXTERNAL_LIB_INCLUDES}
)
target_compile_definitions(${PROJECT_NAME}_knitter
    PRIVATE
    ${COMMON_DEFINES}
    __AVR_ATmega1280__ # should not matter which one is chosen for knitter
)
target_compile_options(${PROJECT_NAME}_knitter PRIVATE
    ${COMMON_FLAGS}
)
target_link_libraries(${PROJECT_NAME}_knitter
    ${COMMON_LINKER_FLAGS}
)
add_dependencies(${PROJECT_NAME}_knitter arduino_mock)

# TODO(sl): Use gtest_discover_tests() ?
# https://cmake.org/cmake/help/git-master/module/GoogleTest.html
enable_testing()
add_test(test_Uno ${PROJECT_NAME}_Uno)
add_test(test_Mega ${PROJECT_NAME}_Mega)
add_test(test_knitter ${PROJECT_NAME}_knitter)
