include(Helpers)

project(WwiseTransfer_Shared)

file(GLOB_RECURSE SHARED_SOURCES
    "${PROJECT_SOURCE_DIR}/*.h"
    "${PROJECT_SOURCE_DIR}/*.cpp")

add_library(${PROJECT_NAME})

find_package(Git REQUIRED)
if(Git_FOUND)
    execute_process(
        COMMAND ${GIT_EXECUTABLE} rev-parse --short HEAD
        RESULT_VARIABLE result
        OUTPUT_VARIABLE COMMIT_HASH)

    execute_process(
        COMMAND ${GIT_EXECUTABLE} rev-parse --abbrev-ref HEAD
        RESULT_VARIABLE result
        OUTPUT_VARIABLE BRANCH_NAME)

    string(REPLACE "\n" "" COMMIT_HASH ${COMMIT_HASH})
    string(REPLACE "\n" "" BRANCH_NAME ${BRANCH_NAME})
endif()

if(DEFINED ENV{VERSION})
    set(VERSION $ENV{VERSION})
else()
    set(VERSION "Unofficial")
endif()

if(DEFINED ENV{BUILD_NUMBER})
    set(BUILD_NUMBER $ENV{BUILD_NUMBER})
else()
    set(BUILD_NUMBER "Local")
endif()

set(VERSION_STRING "${VERSION} ${COMMIT_HASH}")

string(TIMESTAMP year "%Y")
add_compile_definitions(YEAR="${year}")

target_sources(${PROJECT_NAME} PRIVATE ${SHARED_SOURCES})

target_include_directories(${PROJECT_NAME}
    PUBLIC
        ${PROJECT_SOURCE_DIR}/
)

find_package(Wwise REQUIRED)
find_package(AkAutobahn REQUIRED)

file(GLOB RESOURCES "${PROJECT_SOURCE_DIR}/Theme/*/*.*")

juce_add_binary_data(WwiseTransfer_BinaryData
    SOURCES ${RESOURCES})

target_link_libraries(${PROJECT_NAME}
    PUBLIC
        WwiseTransfer_BinaryData
        RapidJSON
        AkAutobahn
        juce::juce_gui_basics
        Wwise
)

target_compile_definitions(${PROJECT_NAME}
    PRIVATE
        JUCE_STANDALONE_APPLICATION=0
    PUBLIC
        JUCE_MANUFACTURER_NAME_STRING="Audiokinetic"
        JUCE_GLOBAL_MODULE_SETTINGS_INCLUDED=1
        JUCE_DISPLAY_SPLASH_SCREEN=0
        JUCE_APPLICATION_VERSION_STRING="${VERSION}"
        BRANCH_NAME="${BRANCH_NAME}"
        BUILD_NUMBER="${BUILD_NUMBER}"
        COMMIT_HASH="${COMMIT_HASH}"
)

source_group(TREE ${PROJECT_SOURCE_DIR} PREFIX "Source Files" FILES ${SHARED_SOURCES})

build_juce_source_groups()