#include "utility.hpp"

#include <fmt/format.h>
#include <vulkan/vk_enum_string_helper.h>

namespace selwonk::vulkan {
void check(VkResult result) {
  if (result != VK_SUCCESS) {
    fmt::println("Detected Vulkan error: {}", string_VkResult(result));
    abort();
  }
}

void check(vk::Result result) { return check(static_cast<VkResult>(result)); }

} // namespace selwonk::vulkan
