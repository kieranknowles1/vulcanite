set(SOURCES
  main.cpp
  core/keyboard.cpp
  core/window.cpp
)

# TODO: Handle WASM
if(NOT ${VN_WASM})
  list(APPEND SOURCES
    impl.cpp
    threadpool.cpp
    core/cvarui.cpp
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
endif()

vn_add_executable(vulcanite ${SOURCES})

# TODO: Handle WASM
if(NOT VN_WASM)
  target_link_libraries(vulcanite PUBLIC
    SDL3::SDL3 # Handled by --use-port for emscripten builds, vn_targets.cmake
  )

  target_link_libraries(vulcanite PUBLIC
    vk-bootstrap
    GPUOpen::VulkanMemoryAllocator
    imgui
    tinyobjloader
    Vulkan::UtilityHeaders
    Vulkan::Vulkan

    vnassets
    nlohmann_json::nlohmann_json
  )
endif()
target_link_libraries(vulcanite PUBLIC
  fmt::fmt

  vncore
)

link_glm(vulcanite)

# TODO: Disable in release builds
if(VN_PROFILED)
  target_compile_definitions(vulcanite PUBLIC TRACY_ENABLE)
  target_link_libraries(vulcanite TracyClient)
endif()

add_dependencies(vulcanite assets)
