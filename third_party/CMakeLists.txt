add_subdirectory(fastgltf)
add_subdirectory(fmt)
add_subdirectory(glm)
add_subdirectory(tracy)

add_library(stb INTERFACE)
target_include_directories(stb INTERFACE stb)

if(NOT VN_WASM)
  add_subdirectory(vk-bootstrap)
endif()

# TODO: Use vendored imgui on nixos builds
add_library(imgui
  imgui/imgui.cpp
)
target_include_directories(imgui PUBLIC imgui)

if(VN_WASM)
  # TODO: Include impl for wasm
else()
  target_sources(imgui PRIVATE imgui/backends/imgui_impl_vulkan.cpp)
  target_link_libraries(imgui PRIVATE Vulkan::Vulkan)
endif()
