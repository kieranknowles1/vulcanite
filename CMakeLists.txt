cmake_minimum_required(VERSION 4.1)
project(vulcanite)

set(CMAKE_CXX_STANDARD 20)

find_package(Vulkan REQUIRED)
find_package(VulkanMemoryAllocator CONFIG REQUIRED)
find_package(SDL3 REQUIRED)
add_subdirectory(third_party)

add_subdirectory(src)
