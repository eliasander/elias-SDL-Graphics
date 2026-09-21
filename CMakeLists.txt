cmake_minimum_required(VERSION 3.31.6)
project(GLFWGame C)

set(CMAKE_C_STANDARD 99)

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


# --- Client ---
file(GLOB SOURCES CONFIGURE_DEPENDS
        src/*.c
        main.c
        glad/src/*.c
        glad/include/*.h
)

add_executable(GLFWGame ${SOURCES}
        library.c)
target_compile_definitions(GLFWGame PRIVATE
        $<$<CONFIG:Debug>:DEBUG>
)

# --- Link libraries ---
target_include_directories(GLFWGame PRIVATE
        src
        include
        glad/src
        glad/include
)
target_link_libraries(GLFWGame PRIVATE
        SDL3::SDL3-static
)

add_custom_command(TARGET GLFWGame POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E rm -rf
            $<TARGET_FILE_DIR:GLFWGame>/shaders
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_SOURCE_DIR}/shaders
            $<TARGET_FILE_DIR:GLFWGame>/shaders
)
