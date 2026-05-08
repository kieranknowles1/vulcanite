set(VN_EMSCRIPTEN_PORTS "--use-port=sdl3")

function(vn_compile_bool TARGET CONTROL)
  if(${${CONTROL}})
    target_compile_definitions(${TARGET} PRIVATE ${CONTROL})
  endif()
endfunction()

# MSVC uses numeric warning IDs instead of names :(
function(vn_warning_error TARGET CLANG_NAME MSVC_ID)
  if(MSVC)
    target_compile_options(${TARGET} PRIVATE /we${MSVC_ID})
  else()
    target_compile_options(${TARGET} PRIVATE -Werror=${CLANG_NAME})
  endif()
endfunction()

function(vn_common_options TARGET)
  target_compile_definitions(
    ${TARGET}
    PRIVATE # Don't define std::vector returning functions
            VULKAN_HPP_DISABLE_ENHANCED_MODE
            # Enable use of designated initializers ({.abc = xyz})
            VULKAN_HPP_NO_CONSTRUCTORS
            # No need to overcomplicate structs with setters/getters
            VULKAN_HPP_NO_SETTERS)

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

  vn_compile_bool(${TARGET} VN_LOGALLOCATIONS)
  vn_compile_bool(${TARGET} VN_LOGCOMPONENTSTATS)
  vn_compile_bool(${TARGET} VN_WASM)
endfunction()

function(vn_add_executable TARGET)
  # TODO: Add headers for visual studio
  add_executable(${TARGET} ${ARGN})
  vn_common_options(${TARGET})

  install(TARGETS ${TARGET})
endfunction()

function(vn_add_library TARGET)
  # TODO: Add headers for visual studio
  add_library(${TARGET} ${ARGN})
  vn_common_options(${TARGET})
endfunction()
