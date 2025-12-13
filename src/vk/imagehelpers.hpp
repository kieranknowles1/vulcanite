#pragma once

#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

namespace selwonk::vk {
class ImageHelpers {
public:
  static void transitionImage(VkCommandBuffer cmd, VkImage img,
                              VkImageLayout currentLayout,
                              VkImageLayout newLayout);

private:
  ImageHelpers() = delete;
};
} // namespace selwonk::vk
