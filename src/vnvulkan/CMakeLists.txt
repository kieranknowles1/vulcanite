set(SOURCES
  vnvulkan/buffer.cpp vnvulkan/buffer.hpp
  vnvulkan/bufferarray.hpp
  vnvulkan/buffermap.hpp
  vnvulkan/impl.cpp
  vnvulkan/shader.cpp vnvulkan/shader.hpp
  vnvulkan/utility.hpp
  vnvulkan/vulkanhandle.cpp vnvulkan/vulkanhandle.hpp
  vnvulkan/vulkaninit.hpp
)

vn_add_library(vnvulkan ${SOURCES})
target_include_directories(vnvulkan PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(vnvulkan PUBLIC
  spdlog
  vk-bootstrap
  GPUOpen::VulkanMemoryAllocator
  Vulkan::UtilityHeaders
  Vulkan::Vulkan

  vncore
  vnassets
  vnecs
  vnsdl
)
