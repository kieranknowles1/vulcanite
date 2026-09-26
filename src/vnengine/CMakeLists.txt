set(SOURCES
  main.cpp
  ecs/camerasystem.cpp ecs/camerasystem.hpp
)

# TODO: Handle WASM
if(NOT ${VN_WASM})
  list(APPEND SOURCES
    ecs/camerapathsystem.cpp ecs/camerapathsystem.hpp
    vk/vulkanengine.cpp vk/vulkanengine.hpp
    ui/cvarui.cpp ui/cvarui.hpp
    ui/element.hpp ui/element.cpp
    ui/profilerui.cpp ui/profilerui.hpp
    ui/sceneviewui.cpp ui/sceneviewui.hpp
  )
endif()

vn_add_executable(vulcanite ${SOURCES})

# TODO: Handle WASM for ImGUI
if(NOT VN_WASM)
  target_link_libraries(vulcanite PUBLIC imgui)
endif()

target_link_libraries(vulcanite PUBLIC
  nlohmann_json::nlohmann_json
  spdlog

  vnassets
  vnecs
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
