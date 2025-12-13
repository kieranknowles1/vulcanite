#pragma once

#include <vulkan/vulkan.h>

namespace selwonk::vul {
// Check the result of a Vulkan function call, and abort if it fails.
// Should be used for all mission-critical operations.
void check(VkResult result);

} // namespace selwonk::vul
