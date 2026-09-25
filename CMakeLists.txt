cmake_minimum_required(VERSION 3.31.6)
project(SnakeSquared C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED True)

include(FetchContent)

# --- SDL3 ---
set(SDL_SHARED OFF CACHE BOOL "" FORCE)
set(SDL_STATIC ON CACHE BOOL "" FORCE)

FetchContent_Declare(
        sdl
        GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
        GIT_TAG release-3.4.16
)

FetchContent_MakeAvailable(sdl)

# --- eos library ---
file(GLOB EOS_SOURCES CONFIGURE_DEPENDS
        src/*.c
        glad/src/*.c
)

add_library(eos STATIC ${EOS_SOURCES})

target_include_directories(eos PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/glad/include
)

target_link_libraries(eos PUBLIC
        SDL3::SDL3-static
)

# --- Client executable ---
add_executable(SnakeSquared main.c)

target_compile_definitions(SnakeSquared PRIVATE
        $<$<CONFIG:Debug>:DEBUG>
)

target_link_libraries(SnakeSquared PRIVATE
        eos
)

add_custom_command(TARGET SnakeSquared POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E rm -rf
            $<TARGET_FILE_DIR:SnakeSquared>/shaders
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_SOURCE_DIR}/shaders
            $<TARGET_FILE_DIR:SnakeSquared>/shaders
)