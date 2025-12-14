#pragma once

#include <vk_mem_alloc.h>
#include <vulkan/vulkan.h>

namespace selwonk::vk {
class VulkanHandle;

struct Image {
  VkImage mImage = nullptr;
  VkImageView mView = nullptr;
  VmaAllocation mAllocation = nullptr;
  VkExtent3D mExtent = {};
  VkFormat mFormat = VK_FORMAT_UNDEFINED;

  void init(VulkanHandle &handle, VkExtent3D extent, VkFormat format,
            VkImageUsageFlags usage);
  void destroy(VulkanHandle &handle);
};
} // namespace selwonk::vk
