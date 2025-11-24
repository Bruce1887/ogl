@echo off
set BUILD_DIR=windows-build

if not exist %BUILD_DIR% (
    mkdir %BUILD_DIR%
)

pushd %BUILD_DIR%

cmake ..
cmake --build . --config Debug

popd
