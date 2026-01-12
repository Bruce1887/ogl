@echo off
:: Set the OpenSSL path explicitly (Ensure you installed the Win64 OpenSSL v3.x.x EXE first)
set OPENSSL_ROOT_DIR=C:/Program Files/OpenSSL-Win64

:: 1. CLEANUP: Delete the old build folder to remove Linux/Unix artifacts
if exist windows_build (
    echo Deleting old build directory...
    rd /s /q windows_build
)

:: 2. CONFIGURE: Generate Visual Studio solution
cmake -B windows_build -G "Visual Studio 18 2026" -A x64 ^
    -DOPENSSL_ROOT_DIR="%OPENSSL_ROOT_DIR%" ^
    -DOPENSSL_USE_STATIC_LIBS=OFF

if %errorlevel% neq 0 (
    echo.
    echo [ERROR] CMake configuration failed. 
    echo Please ensure you installed OpenSSL from https://slproweb.com/products/Win32OpenSSL.html
    echo and that it is located at: %OPENSSL_ROOT_DIR%
    pause
    exit /b %errorlevel%
)

:: 3. BUILD: Compile the code
cmake --build windows_build --config Debug

echo Build completed.