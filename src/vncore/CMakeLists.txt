set(SOURCES
  vncore/bumpallocator.cpp
  vncore/cvar.cpp
  vncore/profiler.cpp
  vncore/vfs.cpp
)

add_library(vncore ${SOURCES})
target_include_directories(vncore PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
