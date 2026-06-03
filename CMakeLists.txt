cmake_minimum_required(VERSION 3.5)

project(
    espelho
    VERSION 0.1.0
    DESCRIPTION "Object replication and stream serialisation, built on Juntos."
    HOMEPAGE_URL "https://github.com/nmstory/espelho"
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED On)
set(CMAKE_CXX_EXTENSIONS Off)

# Pull in juntos (the transport layer) from the submodule
add_subdirectory(submodules/juntos)

# espelho is header-only for now — promote to STATIC once src/*.cpp exist
add_library(espelho_lib INTERFACE)

target_include_directories(
    espelho_lib
    INTERFACE
    "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>"
)

target_link_libraries(espelho_lib INTERFACE juntos_lib)

option(ESPELHO_BUILD_EXAMPLES "Build espelho example programs" ON)
if(ESPELHO_BUILD_EXAMPLES)
    add_subdirectory(examples/position)
endif()
