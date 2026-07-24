include(FetchContent)
FetchContent_Declare(
    SDL2
    GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
    GIT_TAG        release-2.30.11
    GIT_SHALLOW    TRUE
)
set(SDL_TEST OFF CACHE BOOL "" FORCE)
set(SDL_SHARED OFF CACHE BOOL "" FORCE)
set(SDL_STATIC ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(SDL2)

add_executable(espelho_visualiser main.cpp)
target_link_libraries(espelho_visualiser PRIVATE espelho_lib SDL2::SDL2-static)
if(TARGET SDL2::SDL2main)
    target_link_libraries(espelho_visualiser PRIVATE SDL2::SDL2main)
endif()
