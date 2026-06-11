function(add_third_party DIR)
  add_subdirectory(${DIR} EXCLUDE_FROM_ALL)
  foreach(TARGET ${ARGN})
    set_target_properties(${TARGET} PROPERTIES FOLDER third-party)
  endforeach()
endfunction()
# TODO: Could vendored libraries be fetched from vcpkg in emscripten builds?
add_third_party(fastgltf fastgltf)
set(FMT_MODULE OFF)
add_third_party(fmt fmt fmt-c)
add_third_party(glm glm)
add_third_party(tracy TracyClient)
set(SPDLOG_FMT_EXTERNAL ON)
add_third_party(spdlog spdlog)

add_library(stb INTERFACE)
target_include_directories(stb INTERFACE stb)

if(NOT VN_WASM)
  add_third_party(vk-bootstrap vk-bootstrap)
endif()

# TODO: Use vendored imgui on nixos builds
add_library(imgui
  imgui/imgui.cpp
  imgui/imgui_draw.cpp
  imgui/imgui_tables.cpp
  imgui/imgui_widgets.cpp
  imgui/backends/imgui_impl_sdl3.cpp
  imgui/misc/cpp/imgui_stdlib.cpp
  config/imconfig.h
)
target_include_directories(imgui PUBLIC imgui)
target_link_libraries(imgui PUBLIC glm)
set_target_properties(imgui PROPERTIES FOLDER third-party)
target_compile_definitions(imgui PUBLIC "IMGUI_USER_CONFIG=\"${CMAKE_CURRENT_SOURCE_DIR}/config/imconfig.h\"")

if(VN_WASM)
  # TODO: Include impl for wasm
else()
  target_sources(imgui PRIVATE imgui/backends/imgui_impl_vulkan.cpp)
  target_link_libraries(imgui PRIVATE Vulkan::Vulkan)
endif()
