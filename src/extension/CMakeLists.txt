include(Helpers)

if(APPLE)
    macro (set_xcode_property TARGET XCODE_PROPERTY XCODE_VALUE)
        set_property (TARGET ${TARGET} PROPERTY XCODE_ATTRIBUTE_${XCODE_PROPERTY} ${XCODE_VALUE})
    endmacro (set_xcode_property)
endif()

project(ReaWwise)

file(GLOB_RECURSE EXTENSION_SOURCES
    "${PROJECT_SOURCE_DIR}/*.h"
    "${PROJECT_SOURCE_DIR}/*.cpp")

add_library(${PROJECT_NAME} SHARED)

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

target_sources(${PROJECT_NAME} PRIVATE ${EXTENSION_SOURCES})

find_package(Reaper REQUIRED)

target_link_libraries(${PROJECT_NAME}
    PRIVATE
        WwiseTransfer_Shared
        Reaper
)

if(APPLE)
    if(DEFINED ENV{CODE_SIGN_IDENTITY_ID} AND DEFINED ENV{DEVELOPMENT_TEAM_ID})
        SET_XCODE_PROPERTY(${PROJECT_NAME} CODE_SIGN_IDENTITY $ENV{CODE_SIGN_IDENTITY_ID})
        SET_XCODE_PROPERTY(${PROJECT_NAME} DEVELOPMENT_TEAM $ENV{DEVELOPMENT_TEAM_ID})
    endif()

    find_package(Swell REQUIRED)

    target_link_libraries(${PROJECT_NAME}
        PRIVATE
            Swell
    )
endif()

target_compile_definitions(${PROJECT_NAME}
    PRIVATE
        REAPERAPI_MINIMAL
        REAPERAPI_WANT_GetMainHwnd
        REAPERAPI_WANT_ShowConsoleMsg
        REAPERAPI_WANT_AddExtensionsMainMenu
        REAPERAPI_WANT_EnumProjects
        REAPERAPI_WANT_GetSetProjectInfo_String
        REAPERAPI_WANT_GetProjectName
        REAPERAPI_WANT_ResolveRenderPattern
        REAPERAPI_WANT_EnumProjectMarkers2
        REAPERAPI_WANT_EnumRegionRenderMatrix
        REAPERAPI_WANT_GetParentTrack
        REAPERAPI_WANT_GetTrackName
        REAPERAPI_WANT_Main_OnCommand
        REAPERAPI_WANT_GetProjectPathEx
        REAPERAPI_WANT_ShowMessageBox
        REAPERAPI_WANT_GetProjExtState
        REAPERAPI_WANT_SetProjExtState
        REAPERAPI_WANT_MarkProjectDirty
        JUCE_APPLICATION_NAME_STRING="${PROJECT_NAME}"
        JUCE_STANDALONE_APPLICATION=0
)

source_group(TREE ${PROJECT_SOURCE_DIR} PREFIX "Source Files" FILES ${EXTENSION_SOURCES})

build_juce_source_groups()
