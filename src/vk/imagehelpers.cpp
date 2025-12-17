#include "imagehelpers.hpp"
#include "vulkaninit.hpp"

namespace selwonk::vulkan {

void ImageHelpers::transitionImage(vk::CommandBuffer cmd, vk::Image img,
                                   vk::ImageLayout currentLayout,
                                   vk::ImageLayout newLayout) {
  vk::ImageAspectFlags aspectMask =
      (newLayout == vk::ImageLayout::eDepthAttachmentOptimal)
          ? vk::ImageAspectFlagBits::eDepth
          : vk::ImageAspectFlagBits::eColor;

  vk::ImageMemoryBarrier2 barrier = {
      .sType = vk::StructureType::eImageMemoryBarrier2,
      .pNext = nullptr,
      // Bit inefficient, as it stalls the GPU on ALL commands
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

} // namespace selwonk::vulkan
