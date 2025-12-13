#pragma once

#include <vulkan/vulkan.hpp>

namespace selwonk::vul {
class ImageHelpers {
public:
  static void transitionImage(VkCommandBuffer cmd, VkImage img,
                              VkImageLayout currentLayout,
                              VkImageLayout newLayout);

private:
  ImageHelpers() = delete;
};
} // namespace selwonk::vul
