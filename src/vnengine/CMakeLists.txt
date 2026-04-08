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

add_executable(vulcanite ${SOURCES})
target_link_libraries(vulcanite
  vk-bootstrap
  fastgltf
  fmt::fmt
  GPUOpen::VulkanMemoryAllocator
  imgui
  SDL3::SDL3
  stb
  tinyobjloader
  Vulkan::UtilityHeaders
  Vulkan::Vulkan

  vncore
)
target_compile_definitions(vulcanite PUBLIC
  # Don't define std::vector returning functions
  VULKAN_HPP_DISABLE_ENHANCED_MODE
  # Enable use of designated initializers ({.abc = xyz})
  VULKAN_HPP_NO_CONSTRUCTORS
  # No need to overcomplicate structs with setters/getters
  VULKAN_HPP_NO_SETTERS
)
link_glm(vulcanite)
link_json(vulcanite)

# Enable all warnings
target_compile_options(vulcanite PRIVATE -Wall )# -Wextra -Wmost)

# Treat certain warnings as errors
target_compile_options(vulcanite PRIVATE
  # Switch case not handled
  -Werror=switch
  # No return
  -Werror=return-type
  # Unused nodiscard
  -Werror=unused-result
)

if(VN_LOGALLOCATIONS)
  target_compile_definitions(vulcanite PUBLIC VN_LOGALLOCATIONS)
endif()

if(VN_LOGCOMPONENTSTATS)
  target_compile_definitions(vulcanite PUBLIC VN_LOGCOMPONENTSTATS)
endif()

# TODO: Disable in release builds
if(VN_PROFILED)
  target_compile_definitions(vulcanite PUBLIC TRACY_ENABLE)
  target_link_libraries(vulcanite TracyClient)
endif()

add_dependencies(vulcanite assets)
