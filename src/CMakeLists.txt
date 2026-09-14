function(link_glm project)
  target_link_libraries(${project} PUBLIC glm-header-only)
  target_compile_definitions(${project} PUBLIC
    # Vulkan uses the 0..1 range for depth, override GLM's default of -1..1
    GLM_FORCE_DEPTH_ZERO_TO_ONE
    # Allow unstable GLM extensions
    GLM_ENABLE_EXPERIMENTAL
  )
endfunction()

# if(NOT ${VN_WASM})
# endif()
add_subdirectory(vnassets)
add_subdirectory(vncore)
add_subdirectory(vnecs)
add_subdirectory(vnsdl)

if(VN_RENDERER STREQUAL ${VN_RENDER_VULKAN})
  add_subdirectory(vnvulkan)
  set(VN_RENDERERLIB vnvulkan)
elseif(VN_RENDERER STREQUAL ${VN_RENDER_WEBGPU})
  # TODO: Handle WASM
else()
  message(FATAL_ERROR "No renderer for VN_RENDERER")
endif()

add_subdirectory(vnengine)
add_subdirectory(tests)
