#include "image.hpp"

#include "utility.hpp"
#include "vulkanengine.hpp"
#include "vulkanhandle.hpp"
#include "vulkaninit.hpp"
#include <vulkan/vulkan_core.h>

namespace selwonk::vk {
Image::Image(VulkanHandle &handle, VkExtent3D extent, VkFormat format,
             VkImageUsageFlags usage)
    : mExtent(extent), mFormat(format) {

  auto createInfo = VulkanInit::imageCreateInfo(mFormat, usage, mExtent);

  // We want to use GPU memory
  VmaAllocationCreateInfo allocInfo = {
      .usage = VMA_MEMORY_USAGE_GPU_ONLY,
      .requiredFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
  };

  check(vmaCreateImage(handle.mAllocator, &createInfo, &allocInfo, &mImage,
                       &mAllocation, nullptr));
  auto viewInfo = VulkanInit::imageViewCreateInfo(mFormat, mImage,
                                                  VK_IMAGE_ASPECT_COLOR_BIT);
  check(vkCreateImageView(handle.mDevice, &viewInfo, nullptr, &mView));
}

void Image::destroy() {
  auto &handle = VulkanEngine::get().getVulkan();
  vkDestroyImageView(handle.mDevice, mView, nullptr);
  vmaDestroyImage(handle.mAllocator, mImage, mAllocation);
}

} // namespace selwonk::vk
