#include "imagehelpers.hpp"
#include "vulkaninit.hpp"
#include <vulkan/vulkan_core.h>

namespace selwonk::vul {

void ImageHelpers::transitionImage(VkCommandBuffer cmd, VkImage img,
                                   VkImageLayout currentLayout,
                                   VkImageLayout newLayout) {
  VkImageAspectFlags aspectMask =
      (newLayout == VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL)
          ? VK_IMAGE_ASPECT_DEPTH_BIT
          : VK_IMAGE_ASPECT_COLOR_BIT;

  VkImageMemoryBarrier2 barrier = {
      .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
      .pNext = nullptr,
      // Bit inefficient, as it stalls the GPU on ALL commands
      // Would want to be more specific if post-processing
      .srcStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT,
      .srcAccessMask = VK_ACCESS_2_MEMORY_WRITE_BIT,
      .dstStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT,
      .dstAccessMask =
          VK_ACCESS_2_MEMORY_WRITE_BIT | VK_ACCESS_2_MEMORY_READ_BIT,
      .oldLayout = currentLayout,
      .newLayout = newLayout,
      .image = img,
      .subresourceRange = VulkanInit::imageSubresourceRange(aspectMask)};

  VkDependencyInfo depInfo = {.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
                              .pNext = nullptr,
                              .imageMemoryBarrierCount = 1,
                              .pImageMemoryBarriers = &barrier};

  vkCmdPipelineBarrier2(cmd, &depInfo);
}

} // namespace selwonk::vul
