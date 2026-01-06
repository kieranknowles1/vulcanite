#include "image.hpp"

#include <cmath>

#include <cstddef>
#include <fmt/base.h>
#include <memory>
#include <stb_image.h>
#include <stdexcept>
#include <vulkan/vulkan_core.h>

#include "buffer.hpp"
#include "fastgltf/types.hpp"
#include "fastgltf/util.hpp"
#include "imagehelpers.hpp"
#include "utility.hpp"
#include "vulkan/vulkan.hpp"
#include "vulkanhandle.hpp"
#include "vulkaninit.hpp"

namespace selwonk::vulkan {

std::unique_ptr<Image> Image::load(const fastgltf::Asset& asset,
                                   const fastgltf::Image& image) {
  auto data = visitDataSrc(asset, image.data);
  if (data.data == nullptr) {
    fmt::println("Error: {}", stbi_failure_reason());
    throw std::runtime_error("Failed to load image");
  }

  auto img = std::make_unique<Image>(
      vk::Extent3D{data.width, data.height, 1}, vk::Format::eR8G8B8A8Unorm,
      vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eTransferDst);
  img->fill(data.data, data.width * data.height * 4);
  return img;
}

Image::ImgData Image::visitDataSrc(const fastgltf::Asset& asset,
                                   const fastgltf::DataSource& data) {
  return std::visit(
      fastgltf::visitor{
          [](auto& data) -> Image::ImgData {
            throw std::runtime_error("Unsupported image type. Got " +
                                     std::string(typeid(data).name()));
          },
          [&](const fastgltf::sources::Array& array) {
            return loadFromMemory(array.bytes.data(), array.bytes.size_bytes());
          },
          [&](const fastgltf::sources::BufferView& view) {
            auto& bufferView = asset.bufferViews[view.bufferViewIndex];
            auto& buffer = asset.buffers[bufferView.bufferIndex];

            auto bytes = std::visit(
                fastgltf::visitor{
                    [](auto& val) -> const std::byte* {
                      throw std::runtime_error("Unsupported buffer type");
                    },
                    [&](const fastgltf::sources::Array& array) {
                      return array.bytes.data() + bufferView.byteOffset;
                    }},
                buffer.data);
            return loadFromMemory(bytes, bufferView.byteLength);
          }},
      data);
}

Image::ImgData Image::loadFromMemory(const std::byte* bytes, int size) {
  int width;
  int height;
  int channels; // stb_image converts for us, can ignore value
  auto data =
      stbi_load_from_memory(reinterpret_cast<const unsigned char*>(bytes), size,
                            &width, &height, &channels, 4);
  return ImgData{static_cast<uint32_t>(width), static_cast<uint32_t>(height),
                 data};
}

Image::Image(vk::Extent3D extent, vk::Format format, vk::ImageUsageFlags usage,
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

  auto& handle = VulkanHandle::get();
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
