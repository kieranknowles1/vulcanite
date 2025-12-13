#pragma once

#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

namespace selwonk::vk {
// Initialisers for common Vulkan structs
class VulkanInit {
public:
  static VkCommandPoolCreateInfo commandPoolCreateInfo(uint32_t index) {
    return VkCommandPoolCreateInfo{
        .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_GLOBAL_PRIORITY_CREATE_INFO,
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

private:
  VulkanInit() = delete;
};
} // namespace selwonk::vk
