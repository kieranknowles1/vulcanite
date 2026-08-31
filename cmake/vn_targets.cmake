set(VN_EMSCRIPTEN_PORTS "--use-port=sdl3")

function(vn_compile_bool TARGET CONTROL)
  if(${${CONTROL}})
    target_compile_definitions(${TARGET} PRIVATE ${CONTROL})
  endif()
endfunction()

# MSVC uses numeric warning IDs instead of names :(
function(vn_warning_error TARGET CLANG_NAME)
  if(ARGC GREATER 2)
    set(MSVC_ID "${ARGV2}")
  endif()

  if(MSVC)
    target_compile_options(${TARGET} PRIVATE /we${MSVC_ID})
  else()
    target_compile_options(${TARGET} PRIVATE -Werror=${CLANG_NAME})
  endif()
endfunction()

function(vn_common_options TARGET)
  # Enable all warnings
  if(MSVC)
    target_compile_options(${TARGET} PRIVATE /W4)
  else()
    target_compile_options(${TARGET} PRIVATE -Wall) # -Wextra -Wmost)
  endif()

  # Build using the correct encoding
  if(MSVC)
    target_compile_options(${TARGET} PRIVATE /utf-8)
  endif()

  # TODO: emscripten minify options
  if(VN_WASM)
    target_compile_options(${TARGET} PRIVATE ${VN_EMSCRIPTEN_PORTS})
    target_link_options(${TARGET} PRIVATE ${VN_EMSCRIPTEN_PORTS})
  endif()

  # Treat certain warnings as errors Switch case not handled. `default` is
  # omitted for switches that should be exhaustive
  vn_warning_error(${TARGET} switch 4062)
  # Missing break in switch statement
  vn_warning_error(${TARGET} implicit-fallthrough 26819)
  # No return
  vn_warning_error(${TARGET} return-type 4715)
  # Unused nodiscard
  vn_warning_error(${TARGET} unused-result 6031)
  # Move prevents copy elision
  vn_warning_error(${TARGET} pessimizing-move 26479)
  # Order of operations is not left to right
  vn_warning_error(${TARGET} parentheses 4554)
  # Final class is abstract. No dedicated error code under MSVC
  vn_warning_error(${TARGET} abstract-final-class)
  # Unused local variable
  vn_warning_error(${TARGET} unused-variable 4189)

  target_compile_definitions(${TARGET} PRIVATE
          # Don't define std::vector returning functions
          VULKAN_HPP_DISABLE_ENHANCED_MODE
          # Enable use of designated initializers ({.abc = xyz})
          VULKAN_HPP_NO_CONSTRUCTORS
          # No need to overcomplicate structs with setters/getters
          VULKAN_HPP_NO_SETTERS)

  # Group source files in visual studio
  source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${ARGN})

  vn_compile_bool(${TARGET} VN_LOGALLOCATIONS)
  vn_compile_bool(${TARGET} VN_LOGCOMPONENTSTATS)
  # TODO: Remove VN_WASM
  vn_compile_bool(${TARGET} VN_WASM)

  add_compile_definitions(
    "VN_RENDER_VULKAN='${VN_RENDER_VULKAN}'"
    "VN_RENDER_WEBGPU='${VN_RENDER_WEBGPU}'"
  )
  add_compile_definitions("VN_RENDERER='${VN_RENDERER}'")
endfunction()

function(vn_add_executable TARGET)
  add_executable(${TARGET} ${ARGN})
  vn_common_options(${TARGET} ${ARGN})

  install(TARGETS ${TARGET})
endfunction()

function(vn_add_library TARGET)
  add_library(${TARGET} ${ARGN})
  vn_common_options(${TARGET} ${ARGN})
endfunction()
