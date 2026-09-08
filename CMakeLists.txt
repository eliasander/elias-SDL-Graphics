cmake_minimum_required(VERSION 4.3)
project(GLFWGame C)

set(CMAKE_C_STANDARD 99)

include(FetchContent)


# --- raylib ---
FetchContent_Declare(
        glfw
        GIT_REPOSITORY https://github.com/glfw/glfw.git
        GIT_TAG 3.5.1
)

FetchContent_MakeAvailable(glfw)


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
)