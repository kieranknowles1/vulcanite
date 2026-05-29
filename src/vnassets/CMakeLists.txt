set(SOURCES
  vnassets/image.cpp vnassets/image.hpp
  vnassets/impl.cpp
  vnassets/material.hpp
  vnassets/mesh.cpp vnassets/mesh.hpp
  vnassets/sampler.hpp
)

vn_add_library(vnassets ${SOURCES})
target_include_directories(vnassets PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(vnassets PUBLIC
  fastgltf
  stb
  spdlog::spdlog

  vncore
)
