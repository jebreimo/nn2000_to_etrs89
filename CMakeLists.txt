cmake_minimum_required(VERSION 3.30)
project(nn2000_to_wgs84
)

set(CMAKE_CXX_STANDARD 20)

include(FetchContent)
FetchContent_Declare(argos
    GIT_REPOSITORY "https://github.com/jebreimo/Argos.git"
    GIT_TAG "v1.7.2")
FetchContent_Declare(cppembed
    GIT_REPOSITORY "https://github.com/jebreimo/cppembed.git"
    GIT_TAG "v0.1.0")
FetchContent_Declare(gridlib
    GIT_REPOSITORY "https://github.com/jebreimo/GridLib.git"
    GIT_TAG v0.5.1)
FetchContent_MakeAvailable(argos cppembed gridlib)

list(APPEND CMAKE_MODULE_PATH ${cppembed_SOURCE_DIR}/cmake)

include(TargetEmbedCppData)

add_executable(nn2000_to_wgs84
    src/main.cpp
)

target_link_libraries(nn2000_to_wgs84

    PRIVATE
        Argos::Argos
        GridLib::GridLib
)

target_include_directories(Yimage
    PUBLIC
        ${CMAKE_CURRENT_BINARY_DIR}
)

target_embed_cpp_data(nn2000_to_wgs84

    FILES
        src/Geoid.hpp.in
)
