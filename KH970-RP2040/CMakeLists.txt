# rp2040ayab — RP2040 firmware that makes a KH-970 (CSI) knitting machine
# appear to AYAB Desktop as a KH-910/950.
#
# Architecture:
#   core 0 = CSI master bit-bang (timing-critical, csi.c)
#   core 1 = TinyUSB CDC + AYAB API v6 protocol translation (ayab.c, slip.c)
#
# Build:
#   set PICO_SDK_PATH=C:\pico-sdk        (or pass -DPICO_SDK_PATH=...)
#   cmake -G Ninja -B build .
#   cmake --build build
# Output: build/rp2040ayab.uf2

cmake_minimum_required(VERSION 3.13)

include(${PICO_SDK_PATH}/external/pico_sdk_import.cmake)

project(rp2040ayab C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(PICO_BOARD pico)

pico_sdk_init()

add_executable(rp2040ayab
    main.c
    csi.c
    ayab.c
    slip.c
    shared.c
    dbg.c
    usb_descriptors.c
)

target_include_directories(rp2040ayab PRIVATE ${CMAKE_CURRENT_LIST_DIR})

# Debug build: enables a second USB CDC port carrying debug prints.
#   cmake -B build -DRP2040AYAB_DEBUG=ON .
# All debug output is compiled out unless this flag is set.
option(RP2040AYAB_DEBUG "Enable debug logging on the second USB CDC port" OFF)
if(RP2040AYAB_DEBUG)
    target_compile_definitions(rp2040ayab PRIVATE RP2040AYAB_DEBUG=1)
endif()

target_link_libraries(rp2040ayab PRIVATE
    pico_stdlib
    pico_multicore
    tinyusb_device
)

# Emit .uf2 / .bin / .hex
pico_add_extra_outputs(rp2040ayab)
