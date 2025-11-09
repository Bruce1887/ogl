cmake_minimum_required(VERSION 3.5.0)
project(oogabooga VERSION 0.1.0)
cmake_policy(SET CMP0072 NEW)

# --- C++ Standard ---
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# --- Dependencies ---
find_package(OpenGL REQUIRED)
find_package(glfw3 REQUIRED)

# --- Global include directory ---
include_directories(src)

# --- Gather shared/common sources ---
file(GLOB_RECURSE COMMON_SOURCES "src/*.cpp" "src/*.c")
list(FILTER COMMON_SOURCES EXCLUDE REGEX "src/vendor/.*")
list(FILTER COMMON_SOURCES EXCLUDE REGEX "src/glad/.*")
list(FILTER COMMON_SOURCES EXCLUDE REGEX "src/testprograms/.*")
list(FILTER COMMON_SOURCES EXCLUDE REGEX "src/main.cpp")

# --- Vendor libraries ---
add_library(glad src/glad/glad.c)
target_include_directories(glad PUBLIC src)
target_include_directories(glad SYSTEM PRIVATE src/glad)

add_library(stb_image src/vendor/stb_image/stb_image.cpp)
target_include_directories(stb_image PUBLIC src)
target_include_directories(stb_image SYSTEM PRIVATE src/vendor/stb_image)

# --- Common static library (compiled once) ---
add_library(oogabooga_common STATIC ${COMMON_SOURCES})
target_include_directories(oogabooga_common PUBLIC src)
target_link_libraries(oogabooga_common PUBLIC glad stb_image glfw OpenGL::GL)

# --- Function for building executables ---
function(add_gl_executable TARGET_NAME SOURCE_FILE)
    add_executable(${TARGET_NAME} ${SOURCE_FILE})
    target_link_libraries(${TARGET_NAME} PRIVATE oogabooga_common)

    # Compiler warnings
    if (CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        target_compile_options(${TARGET_NAME} PRIVATE -Wall -Wextra -Wpedantic -Wshadow)
    elseif (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
        target_compile_options(${TARGET_NAME} PRIVATE /W4 /permissive-)
    endif()
endfunction()

# --- Automatically add all test programs ---
file(GLOB TEST_PROGRAMS "src/testprograms/*.cpp")
foreach(TEST_SRC ${TEST_PROGRAMS})
    get_filename_component(TEST_NAME ${TEST_SRC} NAME_WE)
    add_gl_executable(${TEST_NAME} ${TEST_SRC})
endforeach()

add_gl_executable(oogabooga src/main.cpp)

# --- (Optional) Install rule ---
# install(TARGETS oogabooga_common RUNTIME DESTINATION bin)
