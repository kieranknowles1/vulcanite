#pragma once

#include "vulkan/vulkan.hpp"
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_core.h>

namespace selwonk::vulkan {
// Initialisers for common Vulkan structs
class VulkanInit {
public:
  static vk::CommandPoolCreateInfo commandPoolCreateInfo(uint32_t index) {
    return vk::CommandPoolCreateInfo{
        // Allow any buffer allocated from the pool to be reset individually
        .flags = vk::CommandPoolCreateFlags::BitsType::eResetCommandBuffer,
        .queueFamilyIndex = index,
    };
  }

  static vk::CommandBufferAllocateInfo bufferAllocateInfo(vk::CommandPool pool,
                                                          uint32_t count = 1) {
    return vk::CommandBufferAllocateInfo{
        .commandPool = pool,
        .level = vk::CommandBufferLevel::ePrimary,
        .commandBufferCount = count,
    };
  }

  static vk::FenceCreateInfo fenceCreateInfo(vk::FenceCreateFlags flags = {}) {
    return vk::FenceCreateInfo{
        .flags = flags,
    };
  }

  static vk::CommandBufferBeginInfo
  commandBufferBeginInfo(vk::CommandBufferUsageFlags flags = {}) {
    return vk::CommandBufferBeginInfo{
        .flags = flags,
        .pInheritanceInfo = nullptr,
    };
  }

  static vk::ImageSubresourceRange
  imageSubresourceRange(vk::ImageAspectFlags aspectMask) {
    return vk::ImageSubresourceRange{
        .aspectMask = aspectMask,
        .baseMipLevel = 0,
        .levelCount = vk::RemainingMipLevels,
        .baseArrayLayer = 0,
        .layerCount = vk::RemainingArrayLayers,
    };
  }

  static vk::SemaphoreSubmitInfo
  semaphoreSubmitInfo(vk::Semaphore semaphore,
                      vk::PipelineStageFlags2 stageMask) {
    return vk::SemaphoreSubmitInfo{
        .semaphore = semaphore,
        .value = 1,
        .stageMask = stageMask,
        .deviceIndex = 0,
    };
  }

  static vk::CommandBufferSubmitInfo
  commandBufferSubmitInfo(vk::CommandBuffer cmd) {
    return vk::CommandBufferSubmitInfo{
        .commandBuffer = cmd,
        .deviceMask = 0, // We're only handling one GPU
    };
  }

  static vk::SubmitInfo2 submitInfo(vk::CommandBufferSubmitInfo *cmd,
                                    vk::SemaphoreSubmitInfo *waitSemaphore,
                                    vk::SemaphoreSubmitInfo *submitSemaphore) {
    return vk::SubmitInfo2{
        .waitSemaphoreInfoCount = waitSemaphore == nullptr ? 0u : 1u,
        .pWaitSemaphoreInfos = waitSemaphore,
        .commandBufferInfoCount = 1,
        .pCommandBufferInfos = cmd,
        .signalSemaphoreInfoCount = submitSemaphore == nullptr ? 0u : 1u,
        .pSignalSemaphoreInfos = submitSemaphore,
    };
  }

  static vk::ImageCreateInfo imageCreateInfo(vk::Format format,
                                             vk::ImageUsageFlags usage,
                                             vk::Extent3D extent) {
    return vk::ImageCreateInfo{
        .imageType = vk::ImageType::e2D,
        .format = format,
        .extent = extent,
        .mipLevels = 1,
        .arrayLayers = 1,
        // Used for MSAA
        .samples = vk::SampleCountFlagBits::e1,
        // Store in whatever format the GPU is optimised for
        // Not suitable if the CPU had to read from it
        .tiling = vk::ImageTiling::eOptimal,
        .usage = usage,
    };
  }

  static vk::ImageViewCreateInfo
  imageViewCreateInfo(vk::Format format, vk::Image image,
                      vk::ImageAspectFlags aspectFlags) {
    return vk::ImageViewCreateInfo{
        .image = image,
        .viewType = vk::ImageViewType::e2D,
        .format = format,
        .subresourceRange =
            {
                .aspectMask = aspectFlags,
                .baseMipLevel = 0,
                .levelCount = 1,
                .baseArrayLayer = 0,
                .layerCount = 1,
            },
    };
  }

  static vk::RenderingAttachmentInfo renderAttachInfo(
      vk::ImageView view, vk::ClearValue *clear,
      vk::ImageLayout layout = vk::ImageLayout::eColorAttachmentOptimal) {
    auto info = vk::RenderingAttachmentInfo{
        .imageView = view,
        .imageLayout = layout,
        .loadOp =
            clear ? vk::AttachmentLoadOp::eClear : vk::AttachmentLoadOp::eLoad,
        .storeOp = vk::AttachmentStoreOp::eStore,
    };
    if (clear)
      info.clearValue = *clear;
    return info;
  }

  static vk::RenderingInfo
  renderInfo(vk::Extent2D extent, vk::RenderingAttachmentInfo *colorAttach,
             vk::RenderingAttachmentInfo *depthAttach) {
    return vk::RenderingInfo{
        .renderArea = vk::Rect2D{vk::Offset2D{0, 0}, extent},
        .layerCount = 1,
        .colorAttachmentCount = 1,
        .pColorAttachments = colorAttach,
        .pDepthAttachment = depthAttach,
        .pStencilAttachment = nullptr,
    };
  }

private:
  VulkanInit() = delete;
};
} // namespace selwonk::vulkan
