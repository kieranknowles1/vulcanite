#include "image.hpp"

#include <cmath>

#include <vulkan/vulkan_core.h>

#include "buffer.hpp"
#include "utility.hpp"
#include "vnassets/image.hpp"
#include "vulkan/vulkan.hpp"
#include "vulkanhandle.hpp"
#include "vulkaninit.hpp"

namespace selwonk::vulkan {

void Image::transition(vk::CommandBuffer cmd, vk::Image img,
                       vk::ImageLayout currentLayout,
                       vk::ImageLayout newLayout) {
  vk::ImageAspectFlags aspectMask =
      (newLayout == vk::ImageLayout::eDepthAttachmentOptimal)
          ? vk::ImageAspectFlagBits::eDepth
          : vk::ImageAspectFlagBits::eColor;

  vk::ImageMemoryBarrier2 barrier = {
      .sType = vk::StructureType::eImageMemoryBarrier2,
      .pNext = nullptr,
      // TODO: Bit inefficient, as it stalls the GPU on ALL commands
      // Would want to be more specific if post-processing
      .srcStageMask = vk::PipelineStageFlagBits2::eAllCommands,
      .srcAccessMask = vk::AccessFlagBits2::eMemoryWrite,
      .dstStageMask = vk::PipelineStageFlagBits2::eAllCommands,
      .dstAccessMask =
          vk::AccessFlagBits2::eMemoryWrite | vk::AccessFlagBits2::eMemoryRead,
      .oldLayout = currentLayout,
      .newLayout = newLayout,
      .image = img,
      .subresourceRange = VulkanInit::imageSubresourceRange(aspectMask)};

  vk::DependencyInfo depInfo = {.imageMemoryBarrierCount = 1,
                                .pImageMemoryBarriers = &barrier};

  cmd.pipelineBarrier2(&depInfo);
}

Image Image::upload(const char* name, const assets::ImageBase::ImgData& data) {
  // TODO: Could we load/upload fewer channels if the image has fewer?
  Image img(
      vk::Extent3D{data.width, data.height, 1}, vk::Format::eR8G8B8A8Unorm,
      vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eTransferDst,
      name);
  img.fill(data.data, data.width * data.height * 4);
  return img;
}

Image::Image(vk::Extent3D extent, vk::Format format, vk::ImageUsageFlags usage,
             const char* name, bool mipmapped) {
  allocate(extent, format, usage, name, mipmapped);
}

void Image::allocate(vk::Extent3D extent, vk::Format format,
                     vk::ImageUsageFlags usage, const char* name,
                     bool mipmapped) {
  auto& handle = VulkanHandle::get();
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

  CHECK(vmaCreateImage(handle.mAllocator, vkUnwrap(createInfo), &allocInfo,
                       vkUnwrap(mImage), &mAllocation, nullptr));
  vmaSetAllocationName(handle.mAllocator, mAllocation, name);
  auto viewInfo = VulkanInit::imageViewCreateInfo(
      mFormat, mImage,
      usage == vk::ImageUsageFlagBits::eDepthStencilAttachment
          ? vk::ImageAspectFlags::BitsType::eDepth
          : vk::ImageAspectFlags::BitsType::eColor);
  CHECK(handle.mDevice.createImageView(&viewInfo, nullptr, &mView));
}

void Image::fill(std::span<const unsigned char> data) {
  assert(data.size_bytes() ==
             bytesPerPixel(mFormat) * mExtent.width * mExtent.height &&
         "Image data size mismatch");

  auto& handle = VulkanHandle::get();
  Buffer stagingBuffer = Buffer::transferBuffer(handle.mAllocator, data.size());
  memcpy(stagingBuffer.getAllocationInfo().pMappedData, data.data(),
         data.size());
  handle.immediateSubmit([&](vk::CommandBuffer cmd) {
    transition(cmd, mImage, vk::ImageLayout::eUndefined,
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
    transition(cmd, mImage, vk::ImageLayout::eTransferDstOptimal,
               vk::ImageLayout::eShaderReadOnlyOptimal);
  });
  stagingBuffer.free(handle.mAllocator);
}

Image::~Image() {
  auto& handle = VulkanHandle::get();
  handle.mDevice.destroyImageView(mView, nullptr);
  vmaDestroyImage(handle.mAllocator, mImage, mAllocation);
}

void Image::copyFromImage(vk::CommandBuffer cmd, const Image& source) {
  copyImpl(cmd, source.mImage, source.mExtent, mImage, mExtent);
}

void Image::copyToSwapchainImage(vk::CommandBuffer cmd, const Image& source,
                                 vk::Image destination, vk::Extent3D extent) {
  copyImpl(cmd, source.mImage, source.mExtent, destination, extent);
}

void Image::copyImpl(vk::CommandBuffer cmd, vk::Image source,
                     vk::Extent3D srcExtent, vk::Image destination,
                     vk::Extent3D dstExtent) {
  VkOffset3D srcOff;
  srcOff.x = srcExtent.width;
  srcOff.y = srcExtent.height;
  srcOff.z = 1;

  VkOffset3D dstOff;
  dstOff.x = dstExtent.width;
  dstOff.y = dstExtent.height;
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
      .srcImage = source,
      .srcImageLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
      .dstImage = destination,
      .dstImageLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
      .regionCount = 1,
      .pRegions = &blitRegion,
      .filter = VK_FILTER_LINEAR,
  };

  vkCmdBlitImage2(cmd, &blitInfo);
}

} // namespace selwonk::vulkan
