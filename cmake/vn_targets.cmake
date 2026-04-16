function(vn_compile_bool TARGET CONTROL)
  if(${${CONTROL}})
    target_compile_definitions(${TARGET} PRIVATE ${CONTROL})
  endif()
endfunction()

function(vn_common_options NAME)
  target_compile_definitions(
    ${NAME}
    PRIVATE # Don't define std::vector returning functions
            VULKAN_HPP_DISABLE_ENHANCED_MODE
            # Enable use of designated initializers ({.abc = xyz})
            VULKAN_HPP_NO_CONSTRUCTORS
            # No need to overcomplicate structs with setters/getters
            VULKAN_HPP_NO_SETTERS)

  # Enable all warnings
  target_compile_options(${NAME} PRIVATE -Wall) # -Wextra -Wmost)

  # TODO: emscripten minify options
  if(${VN_WASM})

  endif()

  # Treat certain warnings as errors
  target_compile_options(
    ${NAME}
    PRIVATE # Switch case not handled
            -Werror=switch
            # Missing break in switch statement
            -Werror=implicit-fallthrough
            # No return
            -Werror=return-type
            # Unused nodiscard
            -Werror=unused-result)

  vn_compile_bool(${NAME} VN_LOGALLOCATIONS)
  vn_compile_bool(${NAME} VN_LOGCOMPONENTSTATS)
  vn_compile_bool(${NAME} VN_WASM)
endfunction()

function(vn_add_executable NAME)
  add_executable(${NAME} ${ARGN})
  vn_common_options(${NAME})
endfunction()

function(vn_add_library NAME)
  add_library(${NAME} ${ARGN})
  vn_common_options(${NAME})
endfunction()
