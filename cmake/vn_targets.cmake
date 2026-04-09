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

  # Treat certain warnings as errors
  target_compile_options(
    ${NAME}
    PRIVATE # Switch case not handled
            -Werror=switch
            # No return
            -Werror=return-type
            # Unused nodiscard
            -Werror=unused-result)

  if(VN_LOGALLOCATIONS)
    target_compile_definitions(${NAME} PRIVATE VN_LOGALLOCATIONS)
  endif()

  if(VN_LOGCOMPONENTSTATS)
    target_compile_definitions(${NAME} PRIVATE VN_LOGCOMPONENTSTATS)
  endif()
endfunction()

function(vn_add_executable NAME)
  add_executable(${NAME} ${ARGN})
  vn_common_options(${NAME})
endfunction()

function(vn_add_library NAME)
  add_library(${NAME} ${ARGN})
  vn_common_options(${NAME})
endfunction()
