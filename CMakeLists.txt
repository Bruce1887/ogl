cmake_minimum_required(VERSION 3.10)
project(oogabooga VERSION 0.1.0)
cmake_policy(SET CMP0072 NEW)

# Allow older submodules (like freetype) to configure
set(CMAKE_POLICY_VERSION_MINIMUM 3.5)

# --- C++ Standard ---
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# --- Dependencies ---
find_package(OpenGL REQUIRED)

add_subdirectory(src/vendor/freetype)
set(FT_DISABLE_HARFBUZZ ON CACHE BOOL "" FORCE)

if(CMAKE_BUILD_TYPE STREQUAL "Debug" OR CMAKE_BUILD_TYPE STREQUAL "debug")
    add_compile_definitions(DEBUG)
endif()

# Platform-specific dependency handling
if(WIN32)
    # Manual GLFW setup for Windows
    set(GLFW_DIR "C:/libs/glfw-3.4.bin.WIN64")
    
    # Create imported target for GLFW
    add_library(glfw SHARED IMPORTED)
    set_target_properties(glfw PROPERTIES
        IMPORTED_LOCATION "${GLFW_DIR}/lib-vc2022/glfw3.dll"
        IMPORTED_IMPLIB "${GLFW_DIR}/lib-vc2022/glfw3dll.lib"
        INTERFACE_INCLUDE_DIRECTORIES "${GLFW_DIR}/include"
    )
else()
    # Linux: use system packages
    find_package(glfw3 REQUIRED)
endif()

# --- Global include directory ---
include_directories(src)

# --- Gather shared/common sources ---
file(GLOB_RECURSE COMMON_SOURCES "src/*.cpp" "src/*.c" "src/*/*.cpp" "src/*/*.c")
# Exclude vendor and main files from the common lib
list(FILTER COMMON_SOURCES EXCLUDE REGEX "src/vendor/.*")
list(FILTER COMMON_SOURCES EXCLUDE REGEX "src/glad/.*")
list(FILTER COMMON_SOURCES EXCLUDE REGEX "src/testprograms/.*")
list(FILTER COMMON_SOURCES EXCLUDE REGEX "src/main.cpp")

# --- Vendor libraries ---

# 1. GLAD
add_library(glad STATIC src/glad/glad.c)
target_include_directories(glad PUBLIC src)
target_include_directories(glad SYSTEM PRIVATE src/glad)

# 2. STB Image
add_library(stb_image STATIC src/vendor/stb_image/stb_image.cpp)
target_include_directories(stb_image PUBLIC src)
target_include_directories(stb_image SYSTEM PRIVATE src/vendor/stb_image)

# 3. HTTP Lib (With OpenSSL Support)
find_package(OpenSSL REQUIRED)
add_library(httplib STATIC src/vendor/httplib/httplib.cpp)

# This enables the SSL code inside httplib.h
target_compile_definitions(httplib PUBLIC CPPHTTPLIB_OPENSSL_SUPPORT)

target_include_directories(httplib
    PUBLIC ${PROJECT_SOURCE_DIR}/src/vendor/httplib
)
# Link OpenSSL libraries to httplib
target_link_libraries(httplib PUBLIC OpenSSL::SSL OpenSSL::Crypto)


# --- Common static library (compiled once) ---
add_library(oogabooga_common STATIC ${COMMON_SOURCES})
target_include_directories(oogabooga_common PUBLIC src)

target_link_libraries(oogabooga_common PUBLIC
    glad
    stb_image
    glfw
    OpenGL::GL
    freetype
    httplib
)


# --- Assimp Configuration ---
set(ASSIMP_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_ASSIMP_TOOLS OFF CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_SAMPLES OFF CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(ASSIMP_INSTALL OFF CACHE BOOL "" FORCE)
set(ASSIMP_WARNINGS_AS_ERRORS OFF CACHE BOOL "" FORCE)
set(ASSIMP_NO_EXPORT ON CACHE BOOL "" FORCE)

if(WIN32)
    set(ASSIMP_BUILD_ZLIB ON CACHE BOOL "" FORCE)
else()
    set(ASSIMP_BUILD_ZLIB OFF CACHE BOOL "" FORCE)
endif()

# Disable specific Assimp importers to speed up build
set(ASSIMP_BUILD_ALL_IMPORTERS_BY_DEFAULT OFF CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_OBJ_IMPORTER ON CACHE BOOL "" FORCE)
# (All other importers default to OFF because of the line above)

add_subdirectory(src/vendor/assimp)
target_link_libraries(oogabooga_common PUBLIC assimp)

# OpenAL - Platform-specific configuration
# Check for Homebrew OpenAL first (preferred on macOS if installed)
set(OPENAL_HOMEBREW_FOUND FALSE)
if(APPLE)
    if(EXISTS "/opt/homebrew/opt/openal-soft/include")
        # Apple Silicon Mac (M1/M2/M3) with Homebrew
        set(OPENAL_INCLUDE_DIR "/opt/homebrew/opt/openal-soft/include")
        set(OPENAL_LIBRARY "/opt/homebrew/opt/openal-soft/lib/libopenal.dylib")
        set(OPENAL_HOMEBREW_FOUND TRUE)
    elseif(EXISTS "/usr/local/opt/openal-soft/include")
        # Intel Mac with Homebrew
        set(OPENAL_INCLUDE_DIR "/usr/local/opt/openal-soft/include")
        set(OPENAL_LIBRARY "/usr/local/opt/openal-soft/lib/libopenal.dylib")
        set(OPENAL_HOMEBREW_FOUND TRUE)
    endif()
endif()

if(OPENAL_HOMEBREW_FOUND)
    # Use Homebrew OpenAL
    message(STATUS "Using Homebrew OpenAL: ${OPENAL_LIBRARY}")
    target_include_directories(oogabooga_common PUBLIC ${OPENAL_INCLUDE_DIR})
    target_link_libraries(oogabooga_common PUBLIC ${OPENAL_LIBRARY})
else()
    # Build OpenAL Soft from submodule (Windows/Linux/macOS without Homebrew)
    message(STATUS "Building OpenAL Soft from submodule")
    set(ALSOFT_EXAMPLES OFF CACHE BOOL "" FORCE)
    set(ALSOFT_UTILS OFF CACHE BOOL "" FORCE)
    set(ALSOFT_TESTS OFF CACHE BOOL "" FORCE)
    set(ALSOFT_BACKEND_ALSA ON CACHE BOOL "" FORCE)
    set(ALSOFT_BACKEND_PULSEAUDIO ON CACHE BOOL "" FORCE)
    set(ALSOFT_BACKEND_PIPEWIRE ON CACHE BOOL "" FORCE)
    set(ALSOFT_BACKEND_WASAPI ON CACHE BOOL "" FORCE)
    set(ALSOFT_BACKEND_WINMM OFF CACHE BOOL "" FORCE)
    set(ALSOFT_BACKEND_COREAUDIO ON CACHE BOOL "" FORCE)
    set(ALSOFT_STL_HARDENING OFF CACHE BOOL "" FORCE)

    add_subdirectory(src/vendor/openal-soft)
    target_link_libraries(oogabooga_common PUBLIC OpenAL)
endif()


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
    
    # Post-build: copy DLLs on Windows
    # I split these into separate commands so if one fails (e.g. static lib), it's clearer
    # Post-build: copy DLLs on Windows    
    if(WIN32)
        # 1. Copy GLFW
        add_custom_command(TARGET ${TARGET_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${GLFW_DIR}/lib-vc2022/glfw3.dll"
            $<TARGET_FILE_DIR:${TARGET_NAME}>
            COMMENT "Copying GLFW DLL..."
        )
        
        # 2. Copy Assimp (only if it is a shared library)
        if(TARGET assimp)
            get_target_property(ASSIMP_TYPE assimp TYPE)
            if(ASSIMP_TYPE STREQUAL "SHARED_LIBRARY")
                add_custom_command(TARGET ${TARGET_NAME} POST_BUILD
                    COMMAND ${CMAKE_COMMAND} -E copy_if_different
                    $<TARGET_FILE:assimp>
                    $<TARGET_FILE_DIR:${TARGET_NAME}>
                    COMMENT "Copying Assimp DLL..."
                )
            endif()
        endif()

        # 3. Copy OpenAL (only if it is a shared library)
        # Note: The target name is "OpenAL" based on your add_subdirectory
        if(TARGET OpenAL)
            get_target_property(OPENAL_TYPE OpenAL TYPE)
            if(OPENAL_TYPE STREQUAL "SHARED_LIBRARY")
                add_custom_command(TARGET ${TARGET_NAME} POST_BUILD
                    COMMAND ${CMAKE_COMMAND} -E copy_if_different
                    $<TARGET_FILE:OpenAL>
                    $<TARGET_FILE_DIR:${TARGET_NAME}>
                    COMMENT "Copying OpenAL DLL..."
                )
            endif()
        endif()

        # 4. Copy OpenSSL DLLs
        if(OPENSSL_FOUND)
            # Hardcoded path is safer for Windows installers
            set(OPENSSL_BIN_DIR "C:/Program Files/OpenSSL-Win64/bin")
            
            file(GLOB OPENSSL_DLLS "${OPENSSL_BIN_DIR}/*.dll")

            if(OPENSSL_DLLS)
                add_custom_command(TARGET ${TARGET_NAME} POST_BUILD
                    COMMAND ${CMAKE_COMMAND} -E copy_if_different
                    ${OPENSSL_DLLS}
                    $<TARGET_FILE_DIR:${TARGET_NAME}>
                    COMMENT "Copying OpenSSL DLLs..."
                )
            else()
                # Fallback: Try to find them relative to the library if the hardcoded path fails
                get_filename_component(OPENSSL_LIB_DIR "${OPENSSL_SSL_LIBRARY}" DIRECTORY)
                # Walk up 4 directories from /lib/VC/x64/MD/ to get to root, then /bin
                file(GLOB OPENSSL_DLLS_FALLBACK "${OPENSSL_LIB_DIR}/../../../../bin/*.dll")
                if(OPENSSL_DLLS_FALLBACK)
                     add_custom_command(TARGET ${TARGET_NAME} POST_BUILD
                        COMMAND ${CMAKE_COMMAND} -E copy_if_different
                        ${OPENSSL_DLLS_FALLBACK}
                        $<TARGET_FILE_DIR:${TARGET_NAME}>
                        COMMENT "Copying OpenSSL DLLs (Fallback)..."
                    )
                endif()
            endif()
        endif()
    endif()

endfunction()

# --- Automatically add all test programs ---
# file(GLOB TEST_PROGRAMS "src/testprograms/*.cpp")
# foreach(TEST_SRC ${TEST_PROGRAMS})
#     get_filename_component(TEST_NAME ${TEST_SRC} NAME_WE)
#     add_gl_executable(${TEST_NAME} ${TEST_SRC})
#     target_compile_options(${TEST_NAME} PRIVATE -g)
# endforeach()

# --- Main executable ---
add_gl_executable(oogabooga src/main.cpp)