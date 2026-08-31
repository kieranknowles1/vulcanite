@echo off
set "BUILD=build"

if not exist %BUILD% mkdir %BUILD%
pushd %BUILD%
cmake .. -DVCPKG_MANIFEST_FEATURES=vulkan
