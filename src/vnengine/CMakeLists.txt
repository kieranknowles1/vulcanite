set(SOURCES
  main.cpp
)

# TODO: Handle WASM
# TODO: Move all vulkan stuff to vnvulkan
if(NOT ${VN_WASM})
  list(APPEND SOURCES
    ecs/camerapathsystem.cpp ecs/camerapathsystem.hpp
    ecs/camerasystem.cpp ecs/camerasystem.hpp
    vk/rendersystem.cpp vk/rendersystem.hpp
    vk/texturemanager.cpp vk/texturemanager.hpp
    vk/vulkandebugrenderer.cpp vk/vulkandebugrenderer.hpp
    vk/vulkanengine.cpp vk/vulkanengine.hpp
    vk/vulkannativehandleprovider.cpp vk/vulkannativehandleprovider.hpp
    ui/cvarui.cpp ui/cvarui.hpp
    ui/profilerui.cpp ui/profilerui.hpp
  )
endif()

vn_add_executable(vulcanite ${SOURCES})

# TODO: Handle WASM
if(NOT VN_WASM)
  target_link_libraries(vulcanite PUBLIC
    imgui

    vnassets
    nlohmann_json::nlohmann_json
    vnecs
  )
endif()
target_link_libraries(vulcanite PUBLIC
  spdlog

  vncore
  vnsdl
  ${VN_RENDERERLIB}
)

link_glm(vulcanite)

# TODO: Disable in release builds
if(VN_PROFILED)
  target_compile_definitions(vulcanite PUBLIC TRACY_ENABLE)
endif()
target_link_libraries(vulcanite PUBLIC TracyClient)

add_dependencies(vulcanite assets)
