#include "utility.hpp"

#include <spdlog/spdlog.h>
#include <vulkan/vk_enum_string_helper.h>

namespace selwonk::vulkan {
void check(VkResult result) {
  // TODO: Use a macro to retain info on file origin
  if (result != VK_SUCCESS) {
    spdlog::critical("Vulkan error: {}", string_VkResult(result));
    abort();
  }
}

void check(vk::Result result) { return check(static_cast<VkResult>(result)); }

} // namespace selwonk::vulkan
