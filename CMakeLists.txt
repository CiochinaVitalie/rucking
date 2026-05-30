# Generated Cmake Pico project file

cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Initialise pico_sdk from installed location
# (note this can come from environment, CMake cache etc)

# == DO NOT EDIT THE FOLLOWING LINES for the Raspberry Pi Pico VS Code Extension to work ==
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 2.2.0)
set(toolchainVersion 14_2_Rel1)
set(picotoolVersion 2.2.0-a4)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================================================================
set(PICO_BOARD pico2 CACHE STRING "Board type")

# Pull in Raspberry Pi Pico SDK (must be before project)
include(pico_sdk_import.cmake)

project(rucking C CXX ASM)

# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()

# Pull in the vendored Ruckig library without building host-side extras.
set(RUCKIG_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/third_party/rucking)
if(NOT EXISTS ${RUCKIG_SOURCE_DIR}/CMakeLists.txt)
    message(FATAL_ERROR
        "Ruckig submodule not found at ${RUCKIG_SOURCE_DIR}. "
        "Run: git submodule update --init --recursive")
endif()

set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(BUILD_PYTHON_MODULE OFF CACHE BOOL "" FORCE)
set(BUILD_CLOUD_CLIENT OFF CACHE BOOL "" FORCE)
set(BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(BUILD_BENCHMARK OFF CACHE BOOL "" FORCE)
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
add_subdirectory(
        ${RUCKIG_SOURCE_DIR}
        ${CMAKE_CURRENT_BINARY_DIR}/third_party/rucking
        EXCLUDE_FROM_ALL)

# Add executable. Default name is the project name, version 0.1

add_executable(rucking rucking.cpp )
 
pico_set_program_name(rucking "rucking")
pico_set_program_version(rucking "0.1")

# Modify the below lines to enable/disable output over UART/USB/RTT
pico_enable_stdio_uart(rucking 1)
pico_enable_stdio_usb(rucking 0)
pico_enable_stdio_rtt(rucking 0)

# Add the standard library to the build
target_link_libraries(rucking
        pico_stdlib
        ruckig::ruckig)

# Add the standard include files to the build
target_include_directories(rucking PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}
)

pico_add_extra_outputs(rucking)
