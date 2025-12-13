#pragma once

#include <vulkan/vulkan.hpp>

namespace selwonk::vul {
// Initialisers for common Vulkan structs
class VulkanInit {
public:
  static vk::CommandPoolCreateInfo commandPoolCreateInfo(uint32_t index) {
    return vk::CommandPoolCreateInfo{
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .pNext = nullptr,
        // Allow any buffer allocated from the pool to be reset individually
        .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
        .queueFamilyIndex = index,
    };
  }

  static vk::CommandBufferAllocateInfo bufferAllocateInfo(vk::CommandPool pool,
                                                          uint32_t count = 1) {
    return vk::CommandBufferAllocateInfo{
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .pNext = nullptr,
        .commandPool = pool,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = count,
    };
  }

  static vk::FenceCreateInfo fenceCreateInfo(vk::FenceCreateFlags flags = 0) {
    return vk::FenceCreateInfo{
        .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
        .pNext = nullptr,
        .flags = flags,
    };
  }

  static vk::SemaphoreCreateInfo
  semaphoreCreateInfo(vk::SemaphoreCreateFlags flags = 0) {
    return vk::SemaphoreCreateInfo{
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
        .pNext = nullptr,
        .flags = flags,
    };
  }

  static vk::CommandBufferBeginInfo
  commandBufferBeginInfo(vk::CommandBufferUsageFlags flags = 0) {
    return vk::CommandBufferBeginInfo{
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .pNext = nullptr,
        .flags = flags,
        .pInheritanceInfo = nullptr,
    };
  }

  static vk::ImageSubresourceRange
  imageSubresourceRange(vk::ImageAspectFlags aspectMask) {
    return vk::ImageSubresourceRange{
        .aspectMask = aspectMask,
        .baseMipLevel = 0,
        .levelCount = VK_REMAINING_MIP_LEVELS,
        .baseArrayLayer = 0,
        .layerCount = VK_REMAINING_ARRAY_LAYERS,
    };
  }

  static vk::SemaphoreSubmitInfo
  semaphoreSubmitInfo(vk::Semaphore semaphore,
                      vk::PipelineStageFlags2 stageMask) {
    return vk::SemaphoreSubmitInfo{
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
        .pNext = nullptr,
        .semaphore = semaphore,
        .value = 1,
        .stageMask = stageMask,
        .deviceIndex = 0,
    };
  }

  static vk::CommandBufferSubmitInfo
  commandBufferSubmitInfo(vk::CommandBuffer cmd) {
    return vk::CommandBufferSubmitInfo{
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO,
        .pNext = nullptr,
        .commandBuffer = cmd,
        .deviceMask = 0, // We're only handling one GPU
    };
  }

  static vk::SubmitInfo2 submitInfo(vk::CommandBufferSubmitInfo *cmd,
                                    vk::SemaphoreSubmitInfo *waitSemaphore,
                                    vk::SemaphoreSubmitInfo *submitSemaphore) {
    return vk::SubmitInfo2{
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
