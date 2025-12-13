set(SOURCES
  main.cpp
  vk/vulkanengine.cpp
)

add_executable(vulcanite ${SOURCES})
target_link_libraries(vulcanite
  vk-bootstrap
  GPUOpen::VulkanMemoryAllocator
  glm
  tinyobjloader
  imgui
  stb
  Vulkan::Vulkan
  SDL3::SDL3
)
