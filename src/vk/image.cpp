#include "image.hpp"

#include "imagehelpers.hpp"
#include "utility.hpp"
#include "vulkan/vulkan.hpp"
#include "vulkanengine.hpp"
#include "vulkanhandle.hpp"
#include "vulkaninit.hpp"
#include <vulkan/vulkan_core.h>

namespace selwonk::vulkan {

void Image::init(VulkanHandle &handle, vk::Extent3D extent, vk::Format format,
                 vk::ImageUsageFlags usage, bool mipmapped) {
  mExtent = extent;
  mFormat = format;

  auto createInfo = VulkanInit::imageCreateInfo(mFormat, usage, mExtent);
  if (mipmapped) {
    createInfo.mipLevels =
        std::floor(std::log2(std::max(extent.width, extent.height))) + 1;
  }

  // We want to use GPU memory
  VmaAllocationCreateInfo allocInfo = {
      .usage = VMA_MEMORY_USAGE_GPU_ONLY,
      .requiredFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
  };

  check(vmaCreateImage(handle.mAllocator, vkUnwrap(createInfo), &allocInfo,
                       vkUnwrap(mImage), &mAllocation, nullptr));
  auto viewInfo = VulkanInit::imageViewCreateInfo(
      mFormat, mImage,
      usage == vk::ImageUsageFlagBits::eDepthStencilAttachment
          ? vk::ImageAspectFlags::BitsType::eDepth
          : vk::ImageAspectFlags::BitsType::eColor);
  check(handle.mDevice.createImageView(&viewInfo, nullptr, &mView));
}

void Image::fill(std::span<const unsigned char> data) {
  assert(data.size_bytes() ==
             bytesPerPixel(mFormat) * mExtent.width * mExtent.height &&
         "Image data size mismatch");

  auto &handle = VulkanEngine::get().getVulkan();
  Buffer stagingBuffer = Buffer::transferBuffer(handle.mAllocator, data.size());
  memcpy(stagingBuffer.getAllocationInfo().pMappedData, data.data(),
         data.size());
  handle.immediateSubmit([&](vk::CommandBuffer cmd) {
    ImageHelpers::transitionImage(cmd, mImage, vk::ImageLayout::eUndefined,
                                  vk::ImageLayout::eTransferDstOptimal);
    vk::BufferImageCopy copyRegion = {
        .bufferOffset = 0,
        .bufferRowLength = 0,
        .bufferImageHeight = 0,
        .imageSubresource =
            {
                .aspectMask = vk::ImageAspectFlagBits::eColor,
                .mipLevel = 0,
                .baseArrayLayer = 0,
                .layerCount = 1,
            },
        .imageExtent = mExtent,
    };
    cmd.copyBufferToImage(stagingBuffer.getBuffer(), mImage,
                          vk::ImageLayout::eTransferDstOptimal, 1, &copyRegion);
    ImageHelpers::transitionImage(cmd, mImage,
                                  vk::ImageLayout::eTransferDstOptimal,
                                  vk::ImageLayout::eShaderReadOnlyOptimal);
  });
  stagingBuffer.free(handle.mAllocator);
}

void Image::destroy(VulkanHandle &handle) {
  vkDestroyImageView(handle.mDevice, mView, nullptr);
  vmaDestroyImage(handle.mAllocator, mImage, mAllocation);
}

void Image::copyFromImage(vk::CommandBuffer cmd, const Image &source) {
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

void Image::copyToSwapchainImage(vk::CommandBuffer cmd, Image source,
                                 vk::Image destination, vk::Extent3D extent) {
  Image tmpDest;
  tmpDest.mImage = destination;
  tmpDest.mExtent = extent;

  tmpDest.copyFromImage(cmd, source);
}

} // namespace selwonk::vulkan
