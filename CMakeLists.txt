cmake_minimum_required(VERSION 4.1)
project(vulcanite)

set(CMAKE_CXX_STANDARD 23)

find_package(Vulkan REQUIRED)
find_package(VulkanUtilityLibraries REQUIRED)
find_package(fmt REQUIRED)
find_package(VulkanMemoryAllocator CONFIG REQUIRED)
find_package(SDL3 REQUIRED)
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

option(VN_LOGALLOCATIONS "Log video memory allocations" OFF)
option(VN_LOGCOMPONENTSTATS "Log component statistics" OFF)

add_subdirectory(assets)
add_subdirectory(src)
