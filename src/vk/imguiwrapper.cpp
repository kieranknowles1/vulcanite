#include "imguiwrapper.hpp"

#include <imgui.h>
#include <imgui_impl_sdl3.h>
#include <imgui_impl_vulkan.h>
#include <vulkan/vulkan_core.h>

#include "utility.hpp"
#include "vulkan/vulkan.hpp"
#include "vulkanhandle.hpp"
#include "vulkaninit.hpp"

namespace selwonk::vulkan {
void ImguiWrapper::init(VulkanHandle &handle, SDL_Window *window) {
  auto createInfo =
      VulkanInit::commandPoolCreateInfo(handle.mGraphicsQueueFamily);
  check(handle.mDevice.createCommandPool(&createInfo, nullptr, &mPool));

  // Place immediate submits in their own buffer
  auto allocInfo = VulkanInit::bufferAllocateInfo(mPool, 1);
  check(handle.mDevice.allocateCommandBuffers(&allocInfo, &mBuffer));

  mFence = handle.createFence(/*signalled=*/true);

  vk::DescriptorPoolSize sizes[] = {
      {vk::DescriptorType::eCombinedImageSampler, 8}};
  vk::DescriptorPoolCreateInfo poolInfo = {
      // Allow descriptor sets to be freed individually and reused
      .flags = vk::DescriptorPoolCreateFlags::BitsType::eFreeDescriptorSet,
      .maxSets = 8,
      .poolSizeCount = 1,
      .pPoolSizes = sizes,
  };

  check(handle.mDevice.createDescriptorPool(&poolInfo, nullptr,
                                            &mDescriptorPool));

  ImGui::CreateContext();
  ImGui_ImplSDL3_InitForVulkan(window);
  auto swapFormat = static_cast<VkFormat>(handle.mSwapchainFormat);
  ImGui_ImplVulkan_InitInfo init = {
      .Instance = handle.mInstance,
      .PhysicalDevice = handle.mPhysicalDevice,
      .Device = handle.mDevice,
      .Queue = handle.mGraphicsQueue,
      .DescriptorPool = mDescriptorPool,
      .MinImageCount = 3,
      .ImageCount = 3,
      .MSAASamples = VK_SAMPLE_COUNT_1_BIT,
      .UseDynamicRendering = true,
      .PipelineRenderingCreateInfo =
          {
              .sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO,
              .colorAttachmentCount = 1,
              .pColorAttachmentFormats = &swapFormat,
          },
  };
  ImGui_ImplVulkan_Init(&init);
  ImGui_ImplVulkan_CreateFontsTexture();
}

void ImguiWrapper::draw(VulkanHandle &handle, vk::CommandBuffer cmd,
                        vk::ImageView target) {
  auto colorAttach = VulkanInit::renderAttachInfo(target, /*clear=*/nullptr);
  auto renderInfo =
      VulkanInit::renderInfo(handle.swapchainExtent2d(), &colorAttach, nullptr);

  cmd.beginRendering(&renderInfo);
  ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), cmd);
  cmd.endRendering();
}

void ImguiWrapper::destroy(VulkanHandle &handle) {
  ImGui_ImplSDL3_Shutdown();
  ImGui_ImplVulkan_Shutdown();
  handle.mDevice.destroyDescriptorPool(mDescriptorPool, nullptr);
  vkDestroyCommandPool(handle.mDevice, mPool, nullptr);
  handle.destroyFence(mFence);
}

void ImguiWrapper::submitImmediate(
    std::function<void(vk::CommandBuffer cmd)> &&function) {
  // TODO: Maybe not needed
}
} // namespace selwonk::vulkan
