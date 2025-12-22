#pragma once

#include <vk_mem_alloc.h>
#include <vulkan/vulkan.hpp>

namespace selwonk::vulkan {
class VulkanHandle;

class Image {
public:
  void init(VulkanHandle &handle, vk::Extent3D extent, vk::Format format,
            vk::ImageUsageFlags usage);
  void destroy(VulkanHandle &handle);

  void copyFromImage(vk::CommandBuffer cmd, const Image &source);
  static void copyToSwapchainImage(vk::CommandBuffer cmd, Image source,
                                   vk::Image destination, vk::Extent3D extent);

  vk::Image getImage() { return mImage; }
  vk::ImageView getView() { return mView; }
  vk::Format getFormat() { return mFormat; }

private:
  vk::Image mImage = nullptr;
  vk::ImageView mView = nullptr;
  VmaAllocation mAllocation = nullptr;
  vk::Extent3D mExtent = {};
  vk::Format mFormat = vk::Format::eUndefined;
};
} // namespace selwonk::vulkan
