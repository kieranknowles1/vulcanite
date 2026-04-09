set(SOURCES
  impl.cpp
  main.cpp
  threadpool.cpp
  core/keyboard.cpp
  core/window.cpp
  ecs/applycommandssystem.cpp
  ecs/camera.cpp
  ecs/camerapathsystem.cpp
  ecs/registry.cpp
  ecs/renderable.cpp
  ecs/transform.cpp
  vk/buffer.cpp
  vk/buffermap.cpp
  vk/camerasystem.cpp
  vk/debug.cpp
  vk/image.cpp
  vk/imguiwrapper.cpp
  vk/mesh.cpp
  vk/meshloader.cpp
  vk/rendersystem.cpp
  vk/samplermanager.cpp
  vk/shader.cpp
  vk/texturemanager.cpp
  vk/utility.cpp
  vk/vulkanengine.cpp
  vk/vulkanhandle.cpp
)

vn_add_executable(vulcanite ${SOURCES})
target_link_libraries(vulcanite
  vk-bootstrap
  fmt::fmt
  GPUOpen::VulkanMemoryAllocator
  imgui
  SDL3::SDL3
  tinyobjloader
  Vulkan::UtilityHeaders
  Vulkan::Vulkan

  vncore
  vnassets
)
link_glm(vulcanite)
link_json(vulcanite)

# TODO: Disable in release builds
if(VN_PROFILED)
  target_compile_definitions(vulcanite PUBLIC TRACY_ENABLE)
  target_link_libraries(vulcanite TracyClient)
endif()

add_dependencies(vulcanite assets)
