include(Helpers)

if(APPLE)
    macro (set_xcode_property TARGET XCODE_PROPERTY XCODE_VALUE)
        set_property (TARGET ${TARGET} PROPERTY XCODE_ATTRIBUTE_${XCODE_PROPERTY} ${XCODE_VALUE})
    endmacro (set_xcode_property)
endif()

project(ReaWwise)

if(WIN32)
    file(GLOB_RECURSE EXTENSION_SOURCES
        "${PROJECT_SOURCE_DIR}/*.h"
        "${PROJECT_SOURCE_DIR}/*.cpp")
else()
    file(GLOB_RECURSE EXTENSION_SOURCES
        "${PROJECT_SOURCE_DIR}/*.h"
        "${PROJECT_SOURCE_DIR}/*.cpp")
endif()

add_library(${PROJECT_NAME} SHARED)
add_library(${PROJECT_NAME}_Static STATIC)

set(PROJECT_LIST ${PROJECT_NAME} ${PROJECT_NAME}_Static)

if(NOT DEFINED ENV{BUILD_NUMBER})
    if(APPLE)
        set(EXTENSION_OUTPUT_DIR "~/Library/Application Support/Reaper/UserPlugins")

        set_target_properties(${PROJECT_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${EXTENSION_OUTPUT_DIR})
        set_target_properties(${PROJECT_NAME} PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${EXTENSION_OUTPUT_DIR})
        set_target_properties(${PROJECT_NAME} PROPERTIES ARCHIVE_OUTPUT_DIRECTORY ${EXTENSION_OUTPUT_DIR})
    elseif(WIN32)
        set(EXTENSION_OUTPUT_DIR "$ENV{APPDATA}\\REAPER\\UserPlugins")

        foreach(OUTPUTCONFIG ${CMAKE_CONFIGURATION_TYPES})
            string(TOUPPER ${OUTPUTCONFIG} OUTPUTCONFIG)
            set_target_properties(${PROJECT_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${EXTENSION_OUTPUT_DIR})
            set_target_properties(${PROJECT_NAME} PROPERTIES LIBRARY_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${EXTENSION_OUTPUT_DIR})
            set_target_properties(${PROJECT_NAME} PROPERTIES ARCHIVE_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${EXTENSION_OUTPUT_DIR})
        endforeach(OUTPUTCONFIG CMAKE_CONFIGURATION_TYPES)

        set_target_properties(${PROJECT_NAME} PROPERTIES VS_DEBUGGER_COMMAND "C:\\Program Files\\REAPER (x64)\\reaper.exe")
    endif()
endif()

set_target_properties(${PROJECT_NAME} PROPERTIES PREFIX "")
set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME reaper_reawwise)

foreach(PROJ ${PROJECT_LIST})
    target_sources(${PROJ} PRIVATE ${EXTENSION_SOURCES})

    target_include_directories(${PROJ}
        PUBLIC
            ${PROJECT_SOURCE_DIR}/
    )

    target_link_libraries(${PROJ}
        PRIVATE
            WwiseTransfer_Shared
            Reaper
    )

    target_compile_definitions(${PROJ}
        PRIVATE
            REAPERAPI_MINIMAL
            REAPERAPI_WANT_GetMainHwnd
            REAPERAPI_WANT_AddExtensionsMainMenu
            REAPERAPI_WANT_EnumProjects
            REAPERAPI_WANT_GetSetProjectInfo_String
            REAPERAPI_WANT_ResolveRenderPattern
            REAPERAPI_WANT_Main_OnCommand
            REAPERAPI_WANT_GetProjExtState
            REAPERAPI_WANT_SetProjExtState
            REAPERAPI_WANT_MarkProjectDirty
            REAPERAPI_WANT_realloc_cmd_register_buf
            REAPERAPI_WANT_realloc_cmd_clear
            REAPERAPI_WANT_GetProjectStateChangeCount
            REAPERAPI_WANT_GetSetProjectInfo
            JUCE_APPLICATION_NAME_STRING="${PROJECT_NAME}"
            JUCE_STANDALONE_APPLICATION=0
            JUCE_REMOVE_COMPONENT_FROM_DESKTOP_ON_WM_DESTROY=1
    )
endforeach()

find_package(Reaper REQUIRED)

if(APPLE)
    if(DEFINED ENV{CODE_SIGN_IDENTITY_ID} AND DEFINED ENV{DEVELOPMENT_TEAM_ID})
        SET_XCODE_PROPERTY(${PROJECT_NAME} CODE_SIGN_IDENTITY $ENV{CODE_SIGN_IDENTITY_ID})
        SET_XCODE_PROPERTY(${PROJECT_NAME} DEVELOPMENT_TEAM $ENV{DEVELOPMENT_TEAM_ID})
    endif()

    find_package(Swell REQUIRED)

    foreach(PROJ ${PROJECT_LIST})
        target_link_libraries(${PROJ} PRIVATE Swell)
    endforeach()
endif()

source_group(TREE ${PROJECT_SOURCE_DIR} PREFIX "Source Files" FILES ${EXTENSION_SOURCES})

build_juce_source_groups()
