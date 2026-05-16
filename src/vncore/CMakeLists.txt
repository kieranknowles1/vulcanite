set(SOURCES
  vncore/cvar.cpp vncore/cvar.hpp
)

# TODO: WASM support
if (NOT VN_WASM)
  list(APPEND SOURCES
    vncore/bumpallocator.cpp vncore/bumpallocator.hpp
    vncore/frustum.cpp vncore/frustum.hpp
    vncore/profiler.cpp vncore/profiler.hpp
    vncore/vfs.cpp vncore/vfs.hpp
    vncore/bumpallocator.hpp
    vncore/chunkedarray.hpp
    vncore/fixedstring.hpp
    vncore/handle.hpp
    vncore/handlelist.hpp
    vncore/math.hpp
    vncore/ringbuffer.hpp
    vncore/singleton.hpp
    vncore/times.hpp
  )
endif()

if(UNIX)
  list(APPEND SOURCES vncore/platform-unix.cpp vncore/platform.hpp)
elseif(WIN32)
  list(APPEND SOURCES vncore/platform-win32.cpp vncore/platform.hpp)
else()
  message(FATAL_ERROR "No platform.cpp for current platform")
endif()

vn_add_library(vncore ${SOURCES})
link_glm(vncore)
# TODO: Move profiler UI and maybe tracy to engine
target_link_libraries(vncore PRIVATE imgui TracyClient)
target_link_libraries(vncore PUBLIC spdlog::spdlog)
target_include_directories(vncore PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
