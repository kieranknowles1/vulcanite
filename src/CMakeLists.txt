set(SOURCES
  main.cpp
  threadpool.cpp
  vfs.cpp
  core/keyboard.cpp
  core/window.cpp
  ecs/registry.cpp
  vk/buffer.cpp
  vk/bumpallocator.cpp
  vk/image.cpp
  vk/imagehelpers.cpp
  vk/imguiwrapper.cpp
  vk/mesh.cpp
  vk/meshloader.cpp
  vk/shader.cpp
  vk/utility.cpp
  vk/vulkanengine.cpp
  vk/vulkanhandle.cpp
)

add_executable(vulcanite ${SOURCES})
target_link_libraries(vulcanite
  vk-bootstrap
  fastgltf
  fmt::fmt
  glm
  GPUOpen::VulkanMemoryAllocator
  imgui
  SDL3::SDL3
  stb
  tinyobjloader
  Vulkan::UtilityHeaders
  Vulkan::Vulkan
)
target_compile_definitions(vulcanite PUBLIC
  # Don't define std::vector returning functions
  VULKAN_HPP_DISABLE_ENHANCED_MODE
  # Enable use of designated initializers ({.abc = xyz})
  VULKAN_HPP_NO_CONSTRUCTORS
  # No need to overcomplicate structs with setters/getters
  VULKAN_HPP_NO_SETTERS
  # Vulkan uses the 0..1 range for depth, override GLM's default of -1..1
  GLM_FORCE_DEPTH_ZERO_TO_ONE
)

# Enable all warnings
target_compile_options(vulcanite PRIVATE -Wall )# -Wextra -Wmost)

if(VN_LOGALLOCATIONS)
  target_compile_definitions(vulcanite PUBLIC VN_LOGALLOCATIONS)
endif()

if(VN_LOGCOMPONENTSTATS)
  target_compile_definitions(vulcanite PUBLIC VN_LOGCOMPONENTSTATS)
endif()

add_dependencies(vulcanite assets)
