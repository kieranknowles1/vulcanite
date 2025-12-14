#pragma once

#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

namespace selwonk::vk {
// Initialisers for common Vulkan structs
class VulkanInit {
public:
  static VkCommandPoolCreateInfo commandPoolCreateInfo(uint32_t index) {
    return VkCommandPoolCreateInfo{
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .pNext = nullptr,
        // Allow any buffer allocated from the pool to be reset individually
        .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
        .queueFamilyIndex = index,
    };
  }

  static VkCommandBufferAllocateInfo bufferAllocateInfo(VkCommandPool pool,
                                                        uint32_t count = 1) {
    return VkCommandBufferAllocateInfo{
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .pNext = nullptr,
        .commandPool = pool,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = count,
    };
  }

  static VkFenceCreateInfo fenceCreateInfo(VkFenceCreateFlags flags = 0) {
    return VkFenceCreateInfo{
        .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
        .pNext = nullptr,
        .flags = flags,
    };
  }

  static VkCommandBufferBeginInfo
  commandBufferBeginInfo(VkCommandBufferUsageFlags flags = 0) {
    return VkCommandBufferBeginInfo{
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .pNext = nullptr,
        .flags = flags,
        .pInheritanceInfo = nullptr,
    };
  }

  static VkImageSubresourceRange
  imageSubresourceRange(VkImageAspectFlags aspectMask) {
    return VkImageSubresourceRange{
        .aspectMask = aspectMask,
        .baseMipLevel = 0,
        .levelCount = VK_REMAINING_MIP_LEVELS,
        .baseArrayLayer = 0,
        .layerCount = VK_REMAINING_ARRAY_LAYERS,
    };
  }

  static VkSemaphoreSubmitInfo
  semaphoreSubmitInfo(VkSemaphore semaphore,
                      VkPipelineStageFlagBits2 stageMask) {
    return VkSemaphoreSubmitInfo{
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
        .pNext = nullptr,
        .semaphore = semaphore,
        .value = 1,
        .stageMask = stageMask,
        .deviceIndex = 0,
    };
  }

  static VkCommandBufferSubmitInfo
  commandBufferSubmitInfo(VkCommandBuffer cmd) {
    return VkCommandBufferSubmitInfo{
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO,
        .pNext = nullptr,
        .commandBuffer = cmd,
        .deviceMask = 0, // We're only handling one GPU
    };
  }

  static VkSubmitInfo2 submitInfo(VkCommandBufferSubmitInfo *cmd,
                                  VkSemaphoreSubmitInfo *waitSemaphore,
                                  VkSemaphoreSubmitInfo *submitSemaphore) {
    return VkSubmitInfo2{
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2,
        .pNext = nullptr,
        .waitSemaphoreInfoCount = waitSemaphore == nullptr ? 0u : 1u,
        .pWaitSemaphoreInfos = waitSemaphore,
        .commandBufferInfoCount = 1,
        .pCommandBufferInfos = cmd,
        .signalSemaphoreInfoCount = submitSemaphore == nullptr ? 0u : 1u,
        .pSignalSemaphoreInfos = submitSemaphore,
    };
  }

  static VkImageCreateInfo
  imageCreateInfo(VkFormat format, VkImageUsageFlags usage, VkExtent3D extent) {
    return VkImageCreateInfo{
        .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
        .pNext = nullptr,
        .imageType = VK_IMAGE_TYPE_2D,
        .format = format,
        .extent = extent,
        .mipLevels = 1,
        .arrayLayers = 1,
        // Used for MSAA
        .samples = VK_SAMPLE_COUNT_1_BIT,
        // Store in whatever format the GPU is optimised for
        // Not suitable if the CPU had to read from it
        .tiling = VK_IMAGE_TILING_OPTIMAL,
        .usage = usage,
    };
  }

  static VkImageViewCreateInfo
  imageViewCreateInfo(VkFormat format, VkImage image,
                      VkImageAspectFlags aspectFlags) {
    return VkImageViewCreateInfo{
        .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        .pNext = nullptr,
        .image = image,
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

private:
  VulkanInit() = delete;
};
} // namespace selwonk::vk
