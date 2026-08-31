@echo off
set "EMSCRIPTEN=../emsdk/"
set "VERSION=6.0.8"
set "BUILD=build-wasm"

call %EMSCRIPTEN%/emsdk_env.bat
call emsdk activate %VERSION%

if not exist %BUILD% mkdir %BUILD%
pushd build-wasm
emcmake cmake .. ^
    "-DVCPKG_CHAINLOAD_TOOLCHAIN_FILE=%EMSDK%/upstream/emscripten/cmake/Modules/Platform/Emscripten.cmake" ^
    -DVCPKG_TARGET_TRIPLET=wasm32-emscripten
popd
