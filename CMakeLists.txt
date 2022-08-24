cmake_minimum_required(VERSION 3.22.0)

project(WwiseTransfer_ALL VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(BUILD_TESTING "" OFF)

if(APPLE)
  set(CMAKE_OSX_ARCHITECTURES "x86_64;arm64")
endif()

set(DEFAULT_BUILD_TYPE "Release")
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    message(STATUS "Setting build type to '${DEFAULT_BUILD_TYPE}' as none was specified.")
    set(CMAKE_BUILD_TYPE "${DEFAULT_BUILD_TYPE}" CACHE
        STRING "Choose the type of build." FORCE)

    # Set the possible values of build type for cmake-gui
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS
        "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
endif()

set(WWISESDK "" CACHE STRING "Path to wwise sdk")

if(WWISESDK STREQUAL "")
    if(DEFINED ENV{WWISESDK})
        set(WWISE_SDK $ENV{WWISESDK})
    else()
        unset(WWISESDK CACHE)
        message(FATAL_ERROR "Must pass in path to Wwise SDK (-DWWISESDK=\"...\") or setup WWISESDK as an environment variable...")
    endif()
else()
    set(WWISE_SDK ${WWISESDK})
endif()

unset(WWISESDK CACHE)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

add_subdirectory(3rd/JUCE)
add_subdirectory(3rd/Catch2)
add_subdirectory(3rd/trompeloeil)
add_subdirectory(src/shared)
add_subdirectory(src/extension)
add_subdirectory(src/standalone)
add_subdirectory(src/test)