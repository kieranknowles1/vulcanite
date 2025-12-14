#include "image.hpp"

#include "utility.hpp"
#include "vulkanhandle.hpp"
#include "vulkaninit.hpp"
#include <vulkan/vulkan_core.h>

namespace selwonk::vk {
void Image::init(VulkanHandle &handle, VkExtent3D extent, VkFormat format,
                 VkImageUsageFlags usage) {
  mExtent = extent;
  mFormat = format;

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

void Image::destroy(VulkanHandle &handle) {
  vkDestroyImageView(handle.mDevice, mView, nullptr);
  vmaDestroyImage(handle.mAllocator, mImage, mAllocation);
}

void Image::copyFromImage(VkCommandBuffer cmd, const Image &source) {
  VkOffset3D srcOff;
  srcOff.x = source.mExtent.width;
  srcOff.y = source.mExtent.height;
  srcOff.z = 1;

  VkOffset3D dstOff;
  dstOff.x = mExtent.width;
  dstOff.y = mExtent.height;
  dstOff.z = 1;

  VkImageSubresourceLayers subresource = {
      .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
      .mipLevel = 0,
      .baseArrayLayer = 0,
      .layerCount = 1,
  };

  VkImageBlit2 blitRegion = {
      .sType = VK_STRUCTURE_TYPE_IMAGE_BLIT_2,
      .pNext = nullptr,
      // Copy full image bounds
      .srcSubresource = subresource,
      .srcOffsets = {{}, srcOff},
      .dstSubresource = subresource,
      .dstOffsets = {{}, dstOff},
  };

  VkBlitImageInfo2 blitInfo = {
      .sType = VK_STRUCTURE_TYPE_BLIT_IMAGE_INFO_2,
      .pNext = nullptr,
      .srcImage = source.mImage,
      .srcImageLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
      .dstImage = mImage,
      .dstImageLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
      .regionCount = 1,
      .pRegions = &blitRegion,
      .filter = VK_FILTER_LINEAR,
  };

  vkCmdBlitImage2(cmd, &blitInfo);
}

void Image::copyToSwapchainImage(VkCommandBuffer cmd, Image source,
                                 VkImage destination, VkExtent3D extent) {
  Image tmpDest;
  tmpDest.mImage = destination;
  tmpDest.mExtent = extent;

  tmpDest.copyFromImage(cmd, source);
}

} // namespace selwonk::vk
