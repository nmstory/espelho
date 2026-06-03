add_executable(espelho_position main.cpp)
target_include_directories(espelho_position PRIVATE .)
target_link_libraries(espelho_position PRIVATE espelho_lib)
