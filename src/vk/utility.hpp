#pragma once

#include <vulkan/vulkan.hpp>

namespace selwonk::vul {
// Check the result of a Vulkan function call, and abort if it fails.
// Should be used for all mission-critical operations.
void check(vk::Result result);

} // namespace selwonk::vul
