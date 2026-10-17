set(SOURCES
  main.cpp
  ecs/camerasystem.cpp ecs/camerasystem.hpp
  engine/vulkanengine.cpp engine/vulkanengine.hpp
  ui/cvarui.cpp ui/cvarui.hpp
  ui/element.hpp ui/element.cpp
  ui/profilerui.cpp ui/profilerui.hpp
  ui/sceneviewui.cpp ui/sceneviewui.hpp
  ui/usageui.cpp ui/usageui.hpp
)

vn_add_executable(vulcanite ${SOURCES})

target_link_libraries(vulcanite PUBLIC
  nlohmann_json::nlohmann_json
  spdlog
  imgui

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
