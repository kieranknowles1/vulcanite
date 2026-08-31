set(SOURCES
  main.cpp
)

# TODO: Handle WASM
if(NOT ${VN_WASM})
  list(APPEND SOURCES
    impl.cpp
    core/cvarui.cpp core/cvarui.hpp
    ecs/camerapathsystem.cpp ecs/camerapathsystem.hpp
    vk/buffer.cpp vk/buffer.hpp
    vk/buffermap.hpp
    vk/camerasystem.cpp vk/camerasystem.hpp
    vk/debug.cpp vk/debug.hpp
    vk/image.cpp vk/image.hpp
    vk/imguiwrapper.cpp vk/imguiwrapper.hpp
    vk/meshloader.cpp vk/meshloader.hpp
    vk/rendersystem.cpp vk/rendersystem.hpp
    vk/samplermanager.cpp vk/samplermanager.hpp
    vk/shader.cpp vk/shader.hpp
    vk/texturemanager.cpp vk/texturemanager.hpp
    vk/utility.hpp
    vk/vulkanengine.cpp vk/vulkanengine.hpp
    vk/vulkanhandle.cpp vk/vulkanhandle.hpp
    vk/bufferarray.hpp
    vk/vulkaninit.hpp
    vk/vulkannativehandleprovider.cpp vk/vulkannativehandleprovider.hpp
  )
endif()

vn_add_executable(vulcanite ${SOURCES})

# TODO: Handle WASM
if(NOT VN_WASM)
  target_link_libraries(vulcanite PUBLIC
    vk-bootstrap
    GPUOpen::VulkanMemoryAllocator
    imgui
    Vulkan::UtilityHeaders
    Vulkan::Vulkan

    vnassets
    nlohmann_json::nlohmann_json
  )
endif()
target_link_libraries(vulcanite PUBLIC
  spdlog

  vncore
  vnsdl
  vnecs
)

link_glm(vulcanite)

# TODO: Disable in release builds
if(VN_PROFILED)
  target_compile_definitions(vulcanite PUBLIC TRACY_ENABLE)
  target_link_libraries(vulcanite TracyClient)
endif()

add_dependencies(vulcanite assets)
