set(SOURCES
  vnassets/impl.hpp
  vnvulkan/buffer.cpp vnvulkan/buffer.hpp
  vnvulkan/image.cpp vnvulkan/image.hpp
  vnvulkan/imguiwrapper.cpp vnvulkan/imguiwrapper.hpp
  vnvulkan/shader.cpp vnvulkan/shader.hpp
  vnvulkan/utility.cpp vnvulkan/utility.hpp
  vnvulkan/vulkanhandle.cpp vnvulkan/vulkanhandle.hpp
  vnvulkan/vulkaninit.hpp
)

vn_add_library(vnvulkan ${SOURCES})
target_include_directories(vnvulkan PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(vnvulkan PUBLIC
  vncore
  vnsdl

  vk-bootstrap
  GPUOpen::VulkanMemoryAllocator
  Vulkan::UtilityHeaders
  Vulkan::Vulkan

  # TODO: Move fastgltf stuff to vnassets (used in image::load)
  fastgltf
  vnassets
)
