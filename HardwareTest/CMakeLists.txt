cmake_minimum_required(VERSION 3.13)

# Pull in SDK (must be before project)
include(pico_sdk_import.cmake)
include(pico_extras_import_optional.cmake)

add_compile_definitions(PICO_XOSC_STARTUP_DELAY_MULTIPLIER=64)

project(HardwareTest C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

if (PICO_SDK_VERSION_STRING VERSION_LESS "1.5.0")
    message(FATAL_ERROR "Raspberry Pi Pico SDK version 1.5.0 (or later) required. Your version is ${PICO_SDK_VERSION_STRING}")
endif()

# Initialize the SDK
pico_sdk_init()

# Pull in the DMX library
include(../lib/Pico-DMX/interfaceLibForPicoSDK.cmake)

add_compile_options(-Wall
        -Wno-format          # int != int32_t as far as the compiler is concerned because gcc has int32_t as long int
        -Wno-unused-function # we have some for the docs that aren't called
        )
if (CMAKE_C_COMPILER_ID STREQUAL "GNU")
    add_compile_options(-Wno-maybe-uninitialized)
endif()

add_executable(HardwareTest)

target_sources(HardwareTest PUBLIC
        main.cpp
)

# pull in common dependencies
target_link_libraries(HardwareTest pico_stdlib picodmx hardware_dma hardware_pio)

# Enable the shell over UART (which is the picoprobe!)
pico_enable_stdio_usb(HardwareTest 1)
pico_enable_stdio_uart(HardwareTest 0)

# create map/bin/hex file etc.
pico_add_extra_outputs(HardwareTest)
