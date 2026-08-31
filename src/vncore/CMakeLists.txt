set(SOURCES
  vncore/cvar.cpp vncore/cvar.hpp
  vncore/vfs.cpp vncore/vfs.hpp
)

# TODO: WASM support
if (NOT VN_WASM)
  list(APPEND SOURCES
    vncore/bumpallocator.cpp vncore/bumpallocator.hpp
    vncore/bumpallocator.hpp
    vncore/chunkedarray.hpp
    vncore/fixedstring.hpp
    vncore/frustum.cpp vncore/frustum.hpp
    vncore/handle.hpp
    vncore/handlelist.hpp
    vncore/math.hpp
    vncore/profiler.cpp vncore/profiler.hpp
    vncore/ringbuffer.hpp
    vncore/singleton.hpp
    vncore/threadpool.cpp vncore/threadpool.hpp
    vncore/times.hpp
    vncore/util.hpp
    vncore/platform.hpp
  )
endif()

# TODO: Single enum for platform everywhere
if(LINUX)
  list(APPEND SOURCES vncore/platform-unix.cpp)
elseif(WIN32)
  list(APPEND SOURCES vncore/platform-win32.cpp)
elseif(VN_WASM)
  # TODO: platform-wasm.cpp
else()
  message(FATAL_ERROR "No platform.cpp for current platform")
endif()

vn_add_library(vncore ${SOURCES})
link_glm(vncore)
# TODO: Move profiler UI and maybe tracy to engine
target_link_libraries(vncore PRIVATE imgui)
target_link_libraries(vncore PUBLIC spdlog::spdlog)
target_include_directories(vncore PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

# TODO: Move profiler UI and maybe tracy to engine
if(VN_PROFILED)
  target_link_libraries(TracyClient)
endif()
