set(SHADERS
  shaders/gradient.comp.hlsl
  shaders/triangle.frag.hlsl
  shaders/triangle.vert.hlsl
)

function(shader_type out_var filename)
  get_filename_component(extension ${filename} EXT)
  if(${extension} STREQUAL ".vert.hlsl")
      set(${out_var} "vs" PARENT_SCOPE)
  elseif(${extension} STREQUAL ".frag.hlsl")
      set(${out_var} "ps" PARENT_SCOPE)
  elseif(${extension} STREQUAL ".comp.hlsl")
      set(${out_var} "cs" PARENT_SCOPE)
  endif()
endfunction()

find_program(DXC dxc)

make_directory(${CMAKE_CURRENT_BINARY_DIR}/shaders)

foreach(SHADER ${SHADERS})
  set(src ${CMAKE_CURRENT_SOURCE_DIR}/${SHADER})
  set(dst ${CMAKE_CURRENT_BINARY_DIR}/${SHADER})
  string(REGEX REPLACE "\\.hlsl$" ".spv" dst ${dst})
  shader_type(type ${src})
  message(${dst})
  add_custom_command(
    OUTPUT ${dst}
    COMMAND ${DXC} -spirv -T ${type}_6_0 ${src} -Fo ${dst}
    DEPENDS ${src}
  )
  list(APPEND SHADER_BINARIES ${dst})
endforeach()

add_custom_target(shaders DEPENDS ${SHADER_BINARIES})
