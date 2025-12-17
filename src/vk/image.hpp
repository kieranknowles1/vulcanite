#pragma once

#include <vk_mem_alloc.h>
#include <vulkan/vulkan.h>

namespace selwonk::vulkan {
class VulkanHandle;

class Image {
public:
  void init(VulkanHandle &handle, VkExtent3D extent, VkFormat format,
            VkImageUsageFlags usage);
  void destroy(VulkanHandle &handle);

  void copyFromImage(VkCommandBuffer cmd, const Image &source);
  static void copyToSwapchainImage(VkCommandBuffer cmd, Image source,
                                   VkImage destination, VkExtent3D extent);

  VkImage getImage() { return mImage; }
  VkImageView getView() { return mView; }

private:
  VkImage mImage = nullptr;
  VkImageView mView = nullptr;
  VmaAllocation mAllocation = nullptr;
  VkExtent3D mExtent = {};
  VkFormat mFormat = VK_FORMAT_UNDEFINED;
};
} // namespace selwonk::vulkan
