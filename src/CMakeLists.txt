function(link_glm project)
  target_link_libraries(${project} PUBLIC glm-header-only)
  target_compile_definitions(${project} PUBLIC
    # Vulkan uses the 0..1 range for depth, override GLM's default of -1..1
    GLM_FORCE_DEPTH_ZERO_TO_ONE
    # Allow unstable GLM extensions
    GLM_ENABLE_EXPERIMENTAL
  )
endfunction()

# TODO: Handle WASM
if(NOT ${VN_WASM})
  add_subdirectory(vnassets)
endif()
add_subdirectory(vncore)
add_subdirectory(vnsdl)
add_subdirectory(vnengine)
add_subdirectory(tests)
