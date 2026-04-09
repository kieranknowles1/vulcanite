set(SOURCES
  vncore/bumpallocator.cpp
  vncore/cvar.cpp
  vncore/frustum.cpp
  vncore/profiler.cpp
  vncore/vfs.cpp
)

if(UNIX)
  list(APPEND SOURCES vncore/platform-unix.cpp)
endif()

vn_add_library(vncore ${SOURCES})
link_glm(vncore)
target_include_directories(vncore PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
