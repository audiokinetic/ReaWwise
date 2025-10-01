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

# Enforce Wwise SDK version 2023 or later
file(READ "${WWISE_SDK}/include/AK/AkWwiseSDKVersion.h" WWISE_VERSION_HEADER)
string(REGEX MATCH "AK_WWISESDK_VERSION_MAJOR[^0-9]*([0-9]+)" _ "${WWISE_VERSION_HEADER}")
set(WWISE_VERSION_MAJOR "${CMAKE_MATCH_1}")

if(NOT DEFINED WWISE_VERSION_MAJOR OR WWISE_VERSION_MAJOR LESS 2023)
    message(FATAL_ERROR "Wwise SDK 2023 or later is required. Found version: ${WWISE_VERSION_MAJOR}")
endif()

message(STATUS "Wwise SDK version: ${WWISE_VERSION_MAJOR}")

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

include_directories(3rd)

# JUCE 6 calls CGWindowListCreateImage which was removed in macOS 15.0.
# ReaWwise does not use window snapshots, so we stub the function out at build time.
# Re-run cmake configure after `git submodule update` to re-apply if the submodule is reset.
if(APPLE)
    find_package(Git REQUIRED)
    set(_juce_patch "${CMAKE_SOURCE_DIR}/cmake/patches/juce_mac_windowing_macos15.patch")
    
    execute_process(
        COMMAND ${GIT_EXECUTABLE} apply --check --ignore-whitespace "${_juce_patch}"
        WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/3rd/JUCE"
        RESULT_VARIABLE _patch_check
        OUTPUT_QUIET ERROR_QUIET
    )
    if(_patch_check EQUAL 0)
        message(STATUS "Applying JUCE macOS 15 compatibility patch...")
        execute_process(
            COMMAND ${GIT_EXECUTABLE} apply --ignore-whitespace "${_juce_patch}"
            WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/3rd/JUCE"
            RESULT_VARIABLE _patch_result
        )
        if(NOT _patch_result EQUAL 0)
            message(FATAL_ERROR "Failed to apply JUCE macOS 15 patch.")
        endif()
    else()
        execute_process(
            COMMAND ${GIT_EXECUTABLE} apply --reverse --check --ignore-whitespace "${_juce_patch}"
            WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/3rd/JUCE"
            RESULT_VARIABLE _patch_reverse_check
            OUTPUT_QUIET ERROR_QUIET
        )
        if(_patch_reverse_check EQUAL 0)
            # Reverse applies cleanly, meaning the patch is already present.
            message(STATUS "JUCE macOS 15 patch already applied — skipping.")
        else()
            message(FATAL_ERROR "JUCE macOS 15 patch cannot be applied (conflicts or missing files).")
        endif()
    endif()
    
    unset(_juce_patch)
    unset(_patch_check)
    unset(_patch_reverse_check)
    unset(_patch_result)
endif()

add_subdirectory(3rd/JUCE)
add_subdirectory(src/shared)
add_subdirectory(src/extension)
add_subdirectory(src/standalone)

set(CMAKE_POLICY_VERSION_MINIMUM 3.5)
add_subdirectory(3rd/Catch2)
add_subdirectory(3rd/trompeloeil)
add_subdirectory(src/test)