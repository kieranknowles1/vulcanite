set(SOURCES
  main.cpp
  threadpool.cpp
  vfs.cpp
  vk/buffer.cpp
  vk/image.cpp
  vk/imagehelpers.cpp
  vk/imguiwrapper.cpp
  vk/mesh.cpp
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
)

add_dependencies(vulcanite shaders)
