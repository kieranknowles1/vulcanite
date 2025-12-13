set(SOURCES
  main.cpp
  vk/vulkanengine.cpp
  vk/vulkanhandle.cpp
  vk/utility.cpp
)

add_executable(vulcanite ${SOURCES})
target_link_libraries(vulcanite
  vk-bootstrap
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
