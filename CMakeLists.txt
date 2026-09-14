cmake_minimum_required(VERSION 3.31.6)
project(GLFWGame C)

set(CMAKE_C_STANDARD 99)

include(FetchContent)

# --- glfw ---

FetchContent_Declare(
        glfw
        GIT_REPOSITORY https://github.com/glfw/glfw.git
        GIT_TAG 3.5.1
)
set(GLFW_BUILD_WAYLAND ON CACHE BOOL "" FORCE)
set(GLFW_BUILD_X11 ON CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)

# --- SDL3 ---
FetchContent_Declare(
        sdl
        GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
        GIT_TAG release-3.4.16
)

FetchContent_MakeAvailable(sdl glfw)


# --- Client ---
file(GLOB SOURCES CONFIGURE_DEPENDS
        src/*.c
        main.c
        glad/src/*.c
        glad/include/*.h
)

add_executable(GLFWGame ${SOURCES})
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
        glfw
        SDL3::SDL3
)

add_custom_command(TARGET GLFWGame POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_SOURCE_DIR}/shaders
            $<TARGET_FILE_DIR:GLFWGame>/shaders
)
