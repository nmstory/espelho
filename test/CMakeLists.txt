include(FetchContent)
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG        v1.14.0
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)  # required for MSVC
FetchContent_MakeAvailable(googletest)

add_executable(espelho_tests
    stream_test.cpp
    replicable_test.cpp
    packet_test.cpp
)

target_link_libraries(espelho_tests PRIVATE espelho_lib GTest::gtest_main)

include(GoogleTest)
gtest_discover_tests(espelho_tests)
