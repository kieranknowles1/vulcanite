#pragma once

#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

namespace selwonk::vul {
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

  static VkSemaphoreCreateInfo
  semaphoreCreateInfo(VkSemaphoreCreateFlags flags = 0) {
    return VkSemaphoreCreateInfo{
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
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

private:
  VulkanInit() = delete;
};
} // namespace selwonk::vul
