set(SOURCES
  vncore/cvar.cpp
)

# TODO: WASM support
if (NOT VN_WASM)
  list(APPEND SOURCES
    vncore/bumpallocator.cpp
    vncore/frustum.cpp
    vncore/profiler.cpp
    vncore/vfs.cpp
  )
endif()

if(UNIX)
  list(APPEND SOURCES vncore/platform-unix.cpp)
endif()

vn_add_library(vncore ${SOURCES})
link_glm(vncore)
target_link_libraries(vncore PRIVATE fmt::fmt)
target_include_directories(vncore PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
