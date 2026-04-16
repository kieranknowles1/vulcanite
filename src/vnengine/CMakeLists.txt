set(SOURCES
  main.cpp
)

# TODO: Handle WASM
if(NOT ${VN_WASM})
  list(APPEND SOURCES
    impl.cpp
    threadpool.cpp
    core/cvarui.cpp
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
endif()

vn_add_executable(vulcanite ${SOURCES})

# TODO: Handle WASM
if(NOT ${VN_WASM})
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
endif()

# TODO: Disable in release builds
if(VN_PROFILED)
  target_compile_definitions(vulcanite PUBLIC TRACY_ENABLE)
  target_link_libraries(vulcanite TracyClient)
endif()

add_dependencies(vulcanite assets)
