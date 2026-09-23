set(SOURCES
  vnvulkan/buffer.cpp vnvulkan/buffer.hpp
  vnvulkan/bufferarray.hpp
  vnvulkan/buffermap.hpp
  vnvulkan/image.cpp vnvulkan/image.hpp
  vnvulkan/imguiwrapper.cpp vnvulkan/imguiwrapper.hpp
  vnvulkan/impl.cpp
  vnvulkan/rendersystem.cpp vnvulkan/rendersystem.hpp
  vnvulkan/samplermanager.cpp vnvulkan/samplermanager.hpp
  vnvulkan/shader.cpp vnvulkan/shader.hpp
  vnvulkan/texturemanager.cpp vnvulkan/texturemanager.hpp
  vnvulkan/utility.hpp
  vnvulkan/vulkandebugrenderer.cpp vnvulkan/vulkandebugrenderer.hpp
  vnvulkan/vulkanhandle.cpp vnvulkan/vulkanhandle.hpp
  vnvulkan/vulkaninit.hpp
  vnvulkan/vulkannativehandleprovider.cpp vnvulkan/vulkannativehandleprovider.hpp
  vnvulkan/vulkanrenderpipeline.cpp vnvulkan/vulkanrenderpipeline.hpp
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
