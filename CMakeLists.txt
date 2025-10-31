cmake_minimum_required(VERSION 3.5.0)
project(oogabooga VERSION 0.1.0)
cmake_policy(SET CMP0072 NEW)
find_package(OpenGL REQUIRED)

# --- Gather common/shared source files (exclude main files and vendor) ---
file(GLOB_RECURSE COMMON_SOURCES "src/*.cpp" "src/*.c")
list(FILTER COMMON_SOURCES EXCLUDE REGEX "src/vendor/.*")
list(FILTER COMMON_SOURCES EXCLUDE REGEX "src/glad/.*")
list(FILTER COMMON_SOURCES EXCLUDE REGEX "src/main.cpp")
list(FILTER COMMON_SOURCES EXCLUDE REGEX "src/testprograms/triangle.cpp")
list(FILTER COMMON_SOURCES EXCLUDE REGEX "src/testprograms/hello_triangle_indexed.cpp")
list(FILTER COMMON_SOURCES EXCLUDE REGEX "src/testprograms/triangle_colorgradient.cpp")

# --- Add vendor code as separate libraries ---
add_library(glad src/glad/glad.c)
target_include_directories(glad PUBLIC src)
target_include_directories(glad SYSTEM PRIVATE src/glad)

add_library(stb_image src/vendor/stb_image/stb_image.cpp)
target_include_directories(stb_image PUBLIC src)
target_include_directories(stb_image SYSTEM PRIVATE src/vendor/stb_image)

# --- Function to create an executable with standard configuration ---
function(add_gl_executable TARGET_NAME SOURCE_FILE)
    add_executable(${TARGET_NAME} ${SOURCE_FILE} ${COMMON_SOURCES})
    target_include_directories(${TARGET_NAME} PRIVATE src)
    target_link_libraries(${TARGET_NAME} PRIVATE glad stb_image glfw OpenGL::GL)
    
    # Add warnings for your own code
    if (CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        target_compile_options(${TARGET_NAME} PRIVATE -Wall -Wextra -Wpedantic -Wshadow)
    elseif (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
        target_compile_options(${TARGET_NAME} PRIVATE /W4 /permissive-)
    endif()
endfunction()

# --- Build executables ---
add_gl_executable(oogabooga src/main.cpp)
add_gl_executable(triangle src/testprograms/triangle.cpp)
add_gl_executable(hello_triangle_indexed src/testprograms/hello_triangle_indexed.cpp)
add_gl_executable(triangle_colorgradient src/testprograms/triangle_colorgradient.cpp)