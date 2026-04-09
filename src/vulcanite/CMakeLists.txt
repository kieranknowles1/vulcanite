set(SOURCES
  main.cpp
)

# TODO: Merge native and wasm builds into this target
add_executable(vulcanite-wasm ${SOURCES})

if(${VN_WASM})
  set(src ${CMAKE_SOURCE_DIR}/emscripten/index.html)
  set(dst ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/index.html)
  add_custom_command(
    TARGET vulcanite-wasm POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy ${src} ${dst}
  )
endif()
