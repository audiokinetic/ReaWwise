add_library(Swell STATIC)

target_sources(Swell PRIVATE "${CMAKE_SOURCE_DIR}/3rd/WDL/WDL/swell/swell-modstub.mm")

#Add a symlink to WDL inside of reaper-sdk
add_custom_command(TARGET Swell PRE_BUILD
    COMMAND ${CMAKE_COMMAND} -E create_symlink ${CMAKE_SOURCE_DIR}/3rd/WDL/WDL ${CMAKE_SOURCE_DIR}/3rd/reaper-sdk/WDL
)

target_compile_definitions(Swell
    PUBLIC
        SWELL_PROVIDED_BY_APP
)

find_library(APPKIT_LIB AppKit)

target_link_libraries(Swell
    PUBLIC
        ${APPKIT_LIB}
)