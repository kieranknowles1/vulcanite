set(SOURCES
  vnwebgpu/webgpurenderpipeline.cpp vnwebgpu/webgpurenderpipeline.cpp
  vnwebgpu/webgpunativehandleprovider.hpp
)

vn_add_library(vnwebgpu ${SOURCES})
target_include_directories(vnwebgpu PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(vnwebgpu PUBLIC
  vncore
  vnassets
  vnecs
)
