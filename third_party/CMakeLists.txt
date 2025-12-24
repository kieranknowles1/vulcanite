add_subdirectory(fastgltf)
add_subdirectory(vk-bootstrap)

add_library(stb INTERFACE)
target_include_directories(stb INTERFACE stb)
