set(SHADERS
  shaders/debug.frag.hlsl
  shaders/debug.vert.hlsl
  shaders/gradient.comp.hlsl
  shaders/triangle.frag.hlsl
  shaders/triangle.vert.hlsl
)
# TODO: Can this be determined automatically?
set(SHADER_HEADERS
  ${CMAKE_CURRENT_SOURCE_DIR}/shaders/gradient.h
  ${CMAKE_CURRENT_SOURCE_DIR}/shaders/interop.h
  ${CMAKE_CURRENT_SOURCE_DIR}/shaders/triangle.h
)

set(MESHES
  meshes/third_party/basicmesh.glb
  meshes/third_party/structure.glb
  meshes/third_party/readme.txt
)

set(PATHS
  paths/default.json
)

set(ICON_ROOT "${CMAKE_SOURCE_DIR}/third_party/mdi/svg")
set(ICON_SIZE 48x48)
set(ICONS
  alert
)

set(SHADER_EXTENSIONS
  SPV_EXT_descriptor_indexing
  SPV_KHR_physical_storage_buffer
)

find_program(DXC dxc REQUIRED)
find_program(MAGICK magick REQUIRED)

set(SHADER_EXTENSION_FLAGS "")
foreach(ext ${SHADER_EXTENSIONS})
  list(APPEND SHADER_EXTENSION_FLAGS "-fspv-extension=${ext}")
endforeach()

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

make_directory(${CMAKE_CURRENT_BINARY_DIR}/shaders)
foreach(SHADER ${SHADERS})
  set(src ${CMAKE_CURRENT_SOURCE_DIR}/${SHADER})
  set(dst ${CMAKE_CURRENT_BINARY_DIR}/${SHADER})
  string(REGEX REPLACE "\\.hlsl$" ".spv" dst ${dst})
  shader_type(type ${src})
  add_custom_command(
    OUTPUT ${dst}
    COMMAND ${DXC} -spirv -T ${type}_6_0 ${src} ${SHADER_EXTENSION_FLAGS} -Fo ${dst}
    DEPENDS ${src} ${SHADER_HEADERS}
  )
  list(APPEND SHADER_BINARIES ${dst})
endforeach()
add_custom_target(shaders DEPENDS ${SHADER_BINARIES})

make_directory(${CMAKE_CURRENT_BINARY_DIR}/textures/icons)
foreach(ICON ${ICONS})
  set(src ${ICON_ROOT}/${ICON}.svg)
  set(dst ${CMAKE_CURRENT_BINARY_DIR}/textures/icons/${ICON}.png)
  add_custom_command(
    OUTPUT ${dst}
    COMMAND ${MAGICK} ${src} -resize ${ICON_SIZE} ${dst}
    DEPENDS ${src}
  )
  list(APPEND ICON_PNGS ${dst})
endforeach()
add_custom_target(icons DEPENDS ${ICON_PNGS})

function(copy_dir TARGET)
  foreach(file ${ARGN})
    set(src ${CMAKE_CURRENT_SOURCE_DIR}/${file})
    set(dst ${CMAKE_CURRENT_BINARY_DIR}/${file})
    add_custom_command(
      OUTPUT ${dst}
      COMMAND ${CMAKE_COMMAND} -E copy ${src} ${dst}
      DEPENDS ${src}
    )
    list(APPEND outputs ${dst})
  endforeach()
  add_custom_target(${TARGET} DEPENDS ${outputs})
endfunction()

copy_dir(meshes ${MESHES})

copy_dir(paths ${PATHS})

add_custom_target(assets DEPENDS shaders icons meshes paths)

if (VN_WASM)
  set(src ${CMAKE_CURRENT_SOURCE_DIR}/emscripten/index.html)
  set(dst ${CMAKE_BINARY_DIR}/index.html)
  add_custom_command(
    OUTPUT ${dst}
    COMMAND ${CMAKE_COMMAND} -E copy ${src} ${dst}
    DEPENDS ${SRC}
  )
  add_custom_target(emscripten-assets DEPENDS ${dst})
  add_dependencies(assets emscripten-assets)
endif()
