cmake_minimum_required(VERSION 4.1)
project(vulcanite)

set(VN_WASM OFF)
if(CMAKE_SYSTEM_NAME STREQUAL "Emscripten")
  set(VN_WASM ON)
elseif(CMAKE_CROSSCOMPILING)
  message(FATAL_ERROR "Unsupported cross-compilation target ${CMAKE_SYSTEM_NAME}")
endif()

# See docs for more information on options
option(VN_LOGALLOCATIONS "Log video memory allocations" OFF)
option(VN_LOGCOMPONENTSTATS "Log component statistics" OFF)
option(VN_PROFILED "Enable profiling" OFF)

set(CMAKE_CXX_STANDARD 23)
list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)
include(vn_targets)

enable_testing()

# TODO: Handle WASM. Nix only provides native libraries
# Need to enable emscripten port of SDL
# SDL3 is provided by the toolchain, JSON and fmt may be easier to build
# Vulkan stuff must be replaced with WebGPU for WASM
if(NOT VN_WASM)
  find_package(Vulkan REQUIRED)
  find_package(VulkanUtilityLibraries REQUIRED)
  find_package(nlohmann_json REQUIRED)
  find_package(VulkanMemoryAllocator CONFIG REQUIRED)
  find_package(SDL3 REQUIRED)
  find_package(GTest REQUIRED)
endif()
add_subdirectory(third_party)

# Required for clangd to detect libraries
set(CMAKE_EXPORT_COMPILE_COMMANDS
    ON
    CACHE INTERNAL "")
if(CMAKE_EXPORT_COMPILE_COMMANDS)
  set(CMAKE_CXX_STANDARD_INCLUDE_DIRECTORIES
      ${CMAKE_CXX_IMPLICIT_INCLUDE_DIRECTORIES})
endif()

# Build executables to the root build directory
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

add_subdirectory(assets)
add_subdirectory(src)
