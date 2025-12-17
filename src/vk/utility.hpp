#pragma once

#include <vulkan/vulkan.hpp>

namespace selwonk::vulkan {
// Check the result of a Vulkan function call, and abort if it fails.
// Should be used for all mission-critical operations.
void check(VkResult result);

void check(vk::Result result);

} // namespace selwonk::vulkan
