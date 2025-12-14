set(SHADERS
  shaders/gradient.comp
)

find_program(DXC dxc)

foreach(SHADER ${SHADERS})
  set(src ${CMAKE_CURRENT_SOURCE_DIR}/${SHADER})
  set(dst ${src}.spv)
  message(${dst})
  add_custom_command(
    OUTPUT ${dst}
    COMMAND ${DXC} -spirv -T cs_6_0 ${src} -Fo ${dst}
    DEPENDS ${src}
  )
  list(APPEND SHADER_BINARIES ${dst})
endforeach()

add_custom_target(shaders DEPENDS ${SHADER_BINARIES})
