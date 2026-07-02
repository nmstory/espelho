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

add_library(espelho_lib STATIC
    src/espelho.cpp
    src/packet_reader.cpp
    src/packet_writer.cpp
    src/type_registry.cpp
)

target_include_directories(
    espelho_lib
    PUBLIC
    "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>"
)

target_link_libraries(espelho_lib PUBLIC juntos_lib)

add_executable(espelho src/main.cpp)
target_link_libraries(espelho PRIVATE espelho_lib)

option(ESPELHO_BUILD_EXAMPLES "Build espelho example programs" ON)
if(ESPELHO_BUILD_EXAMPLES)
    add_subdirectory(alternative-designs/position)
endif()

option(ESPELHO_BUILD_TESTS "Build espelho tests" ON)
if(ESPELHO_BUILD_TESTS)
    enable_testing()
    add_subdirectory(test)
endif()
