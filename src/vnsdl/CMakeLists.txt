set(SOURCES
  vnsdl/keyboard.cpp
  vnsdl/window.cpp
)

vn_add_library(vnsdl ${SOURCES})
target_include_directories(vnsdl PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

if(NOT VN_WASM)
  target_link_libraries(vnsdl PUBLIC
    SDL3::SDL3 # Handled by --use-port for emscripten builds, see vn_targets.cmake
  )
endif()

target_link_libraries(vnsdl PUBLIC
  vncore
  # TODO: Should the platform impl be the one to handle this?
  imgui
)
