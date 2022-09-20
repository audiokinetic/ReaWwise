include(Helpers)

project(WwiseTransfer_Test)

file(GLOB_RECURSE TEST_SOURCES
    "${PROJECT_SOURCE_DIR}/*.h"
    "${PROJECT_SOURCE_DIR}/*.cpp")

add_executable(${PROJECT_NAME})

target_sources(${PROJECT_NAME} PRIVATE ${TEST_SOURCES})

target_link_libraries(${PROJECT_NAME}
    PRIVATE
        WwiseTransfer_Shared
        Reaper
        ReaWwise_Static
    PUBLIC
        Catch2WithMain
        trompeloeil::trompeloeil
)

source_group("Source Files" FILES ${TEST_SOURCES})

build_juce_source_groups()