#pragma once

#include <glm/glm.hpp>
#include <spdlog/spdlog.h>
#include <vulkan/vk_enum_string_helper.h>
#include <vulkan/vulkan.hpp>

#define CHECK(result)                                                          \
  {                                                                            \
    auto& handle = selwonk::vulkan::VulkanHandle::get();                       \
    handle.mCurrentFile = __FILE__;                                            \
    handle.mCurrentLine = __LINE__;                                            \
    handle.mCurrentFunction = __FUNCTION__;                                    \
  }                                                                            \
  if ((VkResult)result != VK_SUCCESS) {                                        \
    SPDLOG_CRITICAL("Vulkan error: {}", string_VkResult((VkResult)result));    \
    abort();                                                                   \
  }

namespace selwonk::vulkan {

// Unwrap a Vulkan native handle from a .hpp wrapper, for use in vma functions
// that require the C-style handle.
template <typename From> typename From::NativeType* vkUnwrap(From& from) {
  // Evil pointer cast, the least cursed part of fast inverse square root
  static_assert(sizeof(From) == sizeof(typename From::NativeType));
  return (typename From::NativeType*)(void*)(&from);
}

inline vk::Extent2D cast(const glm::uvec2& size) {
  return vk::Extent2D{size.x, size.y};
}

} // namespace selwonk::vulkan
