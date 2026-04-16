add_subdirectory(fastgltf)
add_subdirectory(fmt)
add_subdirectory(glm)
add_subdirectory(tracy)

add_library(stb INTERFACE)
target_include_directories(stb INTERFACE stb)

if(NOT VN_WASM)
  add_subdirectory(vk-bootstrap)
endif()
