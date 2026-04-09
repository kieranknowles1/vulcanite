set(SOURCES
  vnassets/image.cpp
  vnassets/impl.cpp
)

vn_add_library(vnassets ${SOURCES})
target_include_directories(vnassets PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(vnassets PUBLIC
  fastgltf
  stb
)
