#pragma once

#include <glm/glm.hpp>
#include <vulkan/vulkan.hpp>

namespace selwonk::vulkan {

// Unwrap a Vulkan native handle from a .hpp wrapper, for use in vma functions
// that require the C-style handle.
template <typename From> typename From::NativeType* vkUnwrap(From& from) {
  // Evil pointer cast, the least cursed part of fast inverse square root
  return (typename From::NativeType*)(void*)(&from);
}

// Check the result of a Vulkan function call, and abort if it fails.
// Should be used for all mission-critical operations.
void check(VkResult result);

void check(vk::Result result);

inline vk::Extent2D cast(const glm::uvec2& size) {
  return vk::Extent2D{size.x, size.y};
}

} // namespace selwonk::vulkan
