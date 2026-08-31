set(SOURCES
  vnecs/applycommandssystem.cpp vnecs/applycommandssystem.hpp
  vnecs/camera.cpp vnecs/camera.hpp
  vnecs/component.hpp
  vnecs/registry.cpp vnecs/registry.hpp
  vnecs/renderable.cpp vnecs/renderable.hpp
  vnecs/transform.cpp vnecs/transform.hpp
  vnecs/component.hpp
  vnecs/entity.hpp
  vnecs/link.hpp
  vnecs/named.hpp
  vnecs/system.hpp
)

vn_add_library(vnecs ${SOURCES})
target_include_directories(vnecs PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(vnecs PUBLIC
  vncore
  vnassets
)
