@echo off
:: Set the OpenSSL path explicitly
set OPENSSL_ROOT_DIR=C:/Program Files/OpenSSL-Win64

:: 1. CLEANUP
if exist windows_build (
    echo Deleting old build directory...
    rd /s /q windows_build
)

:: 2. CONFIGURE: Added -DCMAKE_CXX_FLAGS="/DDEBUG" here
:: This forces "DEBUG" to be defined in the preprocessor.
cmake -B windows_build -G "Visual Studio 18 2026" -A x64 ^
    -DOPENSSL_ROOT_DIR="%OPENSSL_ROOT_DIR%" ^
    -DOPENSSL_USE_STATIC_LIBS=OFF ^
    -DCMAKE_CXX_FLAGS="/DDEBUG"

if %errorlevel% neq 0 (
    echo.
    echo [ERROR] CMake configuration failed. 
    pause
    exit /b %errorlevel%
)

:: 3. BUILD
cmake --build windows_build --config Debug

echo Build completed.