
file(GLOB AK_AUTOBAHN_SOURCES
    "${WWISE_SDK}/samples/WwiseAuthoringAPI/cpp/SampleClient/AkAutobahn/*.h"
    "${WWISE_SDK}/samples/WwiseAuthoringAPI/cpp/SampleClient/AkAutobahn/*.cpp"
)

add_library(AkAutobahn)

set_property(TARGET AkAutobahn PROPERTY CXX_STANDARD 14)

target_sources(AkAutobahn PRIVATE ${AK_AUTOBAHN_SOURCES})

target_include_directories(AkAutobahn
    PUBLIC
        ${WWISE_SDK}/samples/WwiseAuthoringAPI/cpp/SampleClient/AkAutobahn/
)

find_package(RapidJSON REQUIRED)

target_link_libraries(AkAutobahn
    PRIVATE
        RapidJSON
        Wwise
)

target_compile_definitions(AkAutobahn
    PRIVATE
        WIN32_LEAN_AND_MEAN=1
        USE_WEBSOCKET=1
        _CRT_SECURE_NO_WARNINGS=1
)