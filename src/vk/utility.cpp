#include "utility.hpp"

#include <fmt/format.h>
#include <vulkan/vk_enum_string_helper.h>

namespace selwonk::vul {
void check(vk::Result result) {
  if (result != vk::Result::eSuccess) {
    fmt::println("Detected Vulkan error: {}", string_VkResult(result));
    abort();
  }
}
} // namespace selwonk::vul
