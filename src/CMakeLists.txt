function(link_glm project)
  target_link_libraries(${project} glm)
  target_compile_definitions(${project} PUBLIC
    # Vulkan uses the 0..1 range for depth, override GLM's default of -1..1
    GLM_FORCE_DEPTH_ZERO_TO_ONE
    # Allow unstable GLM extensions
    GLM_ENABLE_EXPERIMENTAL
  )
endfunction()

function(link_json project)
  target_link_libraries(${project} nlohmann_json::nlohmann_json)
endfunction()

# TODO: Handle WASM
if(NOT ${VN_WASM})
  add_subdirectory(vnassets)
  add_subdirectory(vncore)
endif()
add_subdirectory(vnengine)
