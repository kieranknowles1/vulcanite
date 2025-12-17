#pragma once

#include <vulkan/vulkan.hpp>

namespace selwonk::vulkan {
class ImageHelpers {
public:
  static void transitionImage(vk::CommandBuffer cmd, vk::Image img,
                              vk::ImageLayout currentLayout,
                              vk::ImageLayout newLayout);

private:
  ImageHelpers() = delete;
};
} // namespace selwonk::vulkan
