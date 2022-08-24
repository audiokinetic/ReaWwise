include(Helpers)

project(WwiseTransfer_Standalone)

file(GLOB_RECURSE STANDALONE_SOURCES
    "${PROJECT_SOURCE_DIR}/*.h"
    "${PROJECT_SOURCE_DIR}/*.cpp")

add_executable(${PROJECT_NAME} WIN32 MACOSX_BUNDLE)

target_sources(${PROJECT_NAME} PRIVATE ${STANDALONE_SOURCES})

target_link_libraries(${PROJECT_NAME}
    PRIVATE
        WwiseTransfer_Shared
)

target_compile_definitions(${PROJECT_NAME}
    PRIVATE
        JUCE_APPLICATION_NAME_STRING="${PROJECT_NAME}"
)

source_group("Source Files" FILES ${STANDALONE_SOURCES})

build_juce_source_groups()
