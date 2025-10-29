cmake_minimum_required(VERSION 3.5.0)
project(newtest VERSION 0.1.0)

cmake_policy(SET CMP0072 NEW)
find_package(OpenGL REQUIRED)

# --- Gather your own source files (exclude vendor and glad) ---
file(GLOB_RECURSE MY_SOURCES
    "src/*.cpp"
    "src/*.c"
)
list(FILTER MY_SOURCES EXCLUDE REGEX "src/vendor/.*")
list(FILTER MY_SOURCES EXCLUDE REGEX "src/glad/.*")

# --- Build your executable with your own files only ---
add_executable(newtest ${MY_SOURCES})
include_directories(src)

# --- Add vendor code as separate libraries ---
add_library(glad src/glad/glad.c)
add_library(stb_image src/vendor/stb_image/stb_image.cpp)

# --- Link everything together ---
target_link_libraries(newtest PRIVATE glad stb_image glfw OpenGL::GL)

# --- Add warnings only for your own code ---
if (CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(newtest PRIVATE -Wall -Wextra -Wpedantic -Wshadow)
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    target_compile_options(newtest PRIVATE /W4 /permissive-)
endif()

# --- Suppress warnings for external/vendor code ---
target_include_directories(glad SYSTEM PRIVATE src/glad)
target_include_directories(stb_image SYSTEM PRIVATE src/vendor/stb_image)
