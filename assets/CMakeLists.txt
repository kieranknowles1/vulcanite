set(SHADERS
  shaders/gradient.comp.hlsl
)

find_program(DXC dxc)

make_directory(${CMAKE_CURRENT_BINARY_DIR}/shaders)

foreach(SHADER ${SHADERS})
  set(src ${CMAKE_CURRENT_SOURCE_DIR}/${SHADER})
  set(dst ${CMAKE_CURRENT_BINARY_DIR}/${SHADER}.spv)
  string(REGEX REPLACE "\\.comp$" ".spv" dst ${dst})
  message(${dst})
  add_custom_command(
    OUTPUT ${dst}
    COMMAND ${DXC} -spirv -T cs_6_0 ${src} -Fo ${dst}
    DEPENDS ${src}
  )
  list(APPEND SHADER_BINARIES ${dst})
endforeach()

add_custom_target(shaders DEPENDS ${SHADER_BINARIES})
