#include "../times.hpp"
#include "imagehelpers.hpp"
#include "shader.hpp"
#include "utility.hpp"
#include "vulkan/vulkan.hpp"
#include "vulkanengine.hpp"
#include "vulkanhandle.hpp"
#include "vulkaninit.hpp"

#include <cassert>
#include <cmath>
#include <cstdint>

#include <SDL3/SDL.h>
#include <SDL3/SDL_init.h>
#include <SDL3/SDL_video.h>
#include <SDL3/SDL_vulkan.h>
#include <VkBootstrap.h>
#include <fmt/base.h>
#include <imgui.h>
#include <imgui_impl_sdl3.h>
#include <imgui_impl_vulkan.h>
#include <memory>

namespace selwonk::vulkan {
VulkanEngine *sEngineInstance = nullptr;
VulkanEngine &VulkanEngine::get() {
  assert(sEngineInstance != nullptr && "Engine instance not initialized");
  return *sEngineInstance;
}

VulkanEngine::VulkanEngine() {}

VulkanEngine::~VulkanEngine() {
  assert(sEngineInstance != this &&
         "Engine must be shut down explicitly before destruction");
}

void VulkanEngine::init(EngineSettings settings) {
  assert(sEngineInstance == nullptr && "Engine cannot be initialised twice");
  sEngineInstance = this;

  fmt::println("Initializing Vulcanite Engine");

  mSettings = settings;

  SDL_Init(SDL_INIT_VIDEO);
  mWindow = SDL_CreateWindow("Vulkanite", mSettings.size.x, mSettings.size.y,
                             SDL_WINDOW_VULKAN);

  mHandle.init(mSettings.mVulkan, mSettings.size, mWindow);

  // No more VkBootstrap - you're on your own now.
  initCommands();
  mImgui.init(mHandle, mWindow);

  // Allocate an image to fill the window
  vk::ImageUsageFlags drawImageUsage = vk::ImageUsageFlagBits::eTransferSrc |
                                       vk::ImageUsageFlagBits::eTransferDst |
                                       vk::ImageUsageFlagBits::eStorage |
                                       vk::ImageUsageFlagBits::eColorAttachment;

  mDrawImage.init(mHandle, {mSettings.size.x, mSettings.size.y, 1},
                  vk::Format::eR16G16B16A16Sfloat, drawImageUsage);
  mDrawExtent = {mSettings.size.x, mSettings.size.y};

  Vfs::Providers providers;
  auto assetDir = Vfs::getExePath().parent_path() / "assets";
  fmt::println("Using asset directory {}", assetDir.c_str());
  providers.push_back(std::make_unique<Vfs::FilesystemProvider>(assetDir));
  mVfs = std::make_unique<Vfs>(std::move(providers));

  initDescriptors();

  fmt::println("Ready to go!");
}

void VulkanEngine::FrameData::init(VulkanHandle &handle) {
  auto poolInfo =
      VulkanInit::commandPoolCreateInfo(handle.mGraphicsQueueFamily);

  // Allocate a pool that will allocate buffers
  check(handle.mDevice.createCommandPool(&poolInfo, nullptr, &mCommandPool));

  // Allocate a default command buffer to submit into
  auto allocInfo = VulkanInit::bufferAllocateInfo(mCommandPool);
  check(handle.mDevice.allocateCommandBuffers(&allocInfo, &mCommandBuffer));

  mSwapchainSemaphore = handle.createSemaphore();

  // Create the fence in the "signalled" state so we can wait on it immediately
  // Simplifies first-frame logic
  auto fenceInfo =
      VulkanInit::fenceCreateInfo(vk::FenceCreateFlags::BitsType::eSignaled);
  mRenderFence = handle.createFence(/*signalled=*/true);
}

void VulkanEngine::FrameData::destroy(VulkanHandle &handle) {
  // Destroying a queue will destroy all its buffers
  handle.mDevice.destroyCommandPool(mCommandPool, nullptr);
  handle.destroySemaphore(mSwapchainSemaphore);
  handle.destroyFence(mRenderFence);
}

void VulkanEngine::initCommands() {
  fmt::println("Initialising command buffers");

  for (auto &buffer : mFrameData) {
    buffer.init(mHandle);
  }
}

void VulkanEngine::initDescriptors() {
  // Allocate a descriptor pool to hold images that compute shaders may write to
  std::array<DescriptorAllocator::PoolSizeRatio, 1> sizes = {
      {vk::DescriptorType::eStorageImage, 1}};

  // Reserve space for 10 such descriptors
  mGlobalDescriptorAllocator.init(10, sizes);

  // Allocate one of these descriptors
  DescriptorLayoutBuilder builder;
  builder.addBinding(0, sizes[0].type);
  mDrawImageDescriptorLayout =
      builder.build(mHandle.mDevice, vk::ShaderStageFlags::BitsType::eCompute);
  mDrawImageDescriptors =
      mGlobalDescriptorAllocator.allocate(mDrawImageDescriptorLayout);

  // Point the descriptor to the draw image
  vk::DescriptorImageInfo info = {
      .imageView = mDrawImage.getView(),
      .imageLayout = vk::ImageLayout::eGeneral,
  };

  // Write to the pool
  vk::WriteDescriptorSet write = {
      .dstSet = mDrawImageDescriptors,
      .dstBinding = 0,
      .descriptorCount = 1,
      .descriptorType = sizes[0].type,
      .pImageInfo = &info,
  };
  mHandle.mDevice.updateDescriptorSets(1, &write, 0, nullptr);

  ShaderStage stage("gradient.comp.spv",
                    vk::ShaderStageFlags::BitsType::eCompute, "main");
  mGradientShader.link(mDrawImageDescriptorLayout, stage,
                       sizeof(GradientPushConstants));
}

void VulkanEngine::run() {
  SDL_Event e;
  bool quit = false;

  while (!quit) {
    while (SDL_PollEvent(&e)) {
      if (e.type == SDL_EVENT_QUIT) {
        quit = true;
      }
      ImGui_ImplSDL3_ProcessEvent(&e);
    }

    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplSDL3_NewFrame();
    ImGui::NewFrame();
    ImGui::ShowDemoWindow();
    ImGui::Render();
    draw();
  }
}

void VulkanEngine::drawBackground(vk::CommandBuffer cmd) {
  cmd.bindPipeline(vk::PipelineBindPoint::eCompute, mGradientShader.mPipeline);
  cmd.bindDescriptorSets(vk::PipelineBindPoint::eCompute,
                         mGradientShader.mLayout, /*firstSet=*/0,
                         /*descriptorSetCount=*/1, &mDrawImageDescriptors,
                         /*dynamicOffsetCount=*/0,
                         /*pDynamicOffsets=*/nullptr);

  cmd.pushConstants(mGradientShader.mLayout,
                    vk::ShaderStageFlags::BitsType::eCompute, 0,
                    sizeof(GradientPushConstants), &mPushConstants);

  const int workgroupSize = 16;
  vkCmdDispatch(cmd, std::ceil(mDrawExtent.width / workgroupSize),
                std::ceil(mDrawExtent.height / workgroupSize), 1);
}

void VulkanEngine::draw() {
  auto &frame = getCurrentFrame();
  auto timeout = millisToNanoSeconds(1000);

  // Wait for the previous frame to finish
  check(mHandle.mDevice.waitForFences(1, &frame.mRenderFence, true, timeout));
  check(mHandle.mDevice.resetFences(1, &frame.mRenderFence));

  // Request a buffer to draw to
  uint32_t swapchainImageIndex;
  check(vkAcquireNextImageKHR(mHandle.mDevice, mHandle.mSwapchain, timeout,
                              frame.mSwapchainSemaphore, nullptr,
                              &swapchainImageIndex));
  auto &swapchainEntry = mHandle.mSwapchainEntries[swapchainImageIndex];

  auto cmd = frame.mCommandBuffer;
  // We're certain the command buffer is not in use, prepare for recording
  check(vkResetCommandBuffer(cmd, 0));
  // We won't be submitting the buffer multiple times in a row, let Vulkan know
  // Drivers may be able to get a small speed boost
  auto beginInfo = VulkanInit::commandBufferBeginInfo(
      vk::CommandBufferUsageFlags::BitsType::eOneTimeSubmit);
  check(cmd.begin(&beginInfo));

  // Make the draw image writable, we don't care about destroying previous
  // data
  ImageHelpers::transitionImage(cmd, mDrawImage.getImage(),
                                vk::ImageLayout::eUndefined,
                                vk::ImageLayout::eGeneral);

  drawBackground(cmd);

  // Make the draw image readable again
  ImageHelpers::transitionImage(cmd, mDrawImage.getImage(),
                                vk::ImageLayout::eGeneral,
                                vk::ImageLayout::eTransferSrcOptimal);

  // Copy draw image to the swapchain
  ImageHelpers::transitionImage(cmd, swapchainEntry.image,
                                vk::ImageLayout::eUndefined,
                                vk::ImageLayout::eTransferDstOptimal);
  Image::copyToSwapchainImage(cmd, mDrawImage, swapchainEntry.image,
                              mHandle.mSwapchainExtent);

  ImageHelpers::transitionImage(cmd, swapchainEntry.image,
                                vk::ImageLayout::eTransferDstOptimal,
                                vk::ImageLayout::eAttachmentOptimal);
  // Draw directly to the swapchain, which matches the format ImGui expects
  mImgui.draw(mHandle, cmd, swapchainEntry.view);
  ImageHelpers::transitionImage(cmd, swapchainEntry.image,
                                vk::ImageLayout::eAttachmentOptimal,
                                vk::ImageLayout::ePresentSrcKHR);

  // Finalise the command buffer, ready for execution
  check(vkEndCommandBuffer(cmd));

  // Submit, after all this time
  auto cmdInfo = VulkanInit::commandBufferSubmitInfo(cmd);
  auto waitInfo = VulkanInit::semaphoreSubmitInfo(
      frame.mSwapchainSemaphore,
      vk::PipelineStageFlags2::BitsType::eColorAttachmentOutput);
  auto signalInfo = VulkanInit::semaphoreSubmitInfo(
      swapchainEntry.semaphore,
      vk::PipelineStageFlags2::BitsType::eAllGraphics);
  auto submit = VulkanInit::submitInfo(&cmdInfo, &waitInfo, &signalInfo);
  // Execute
  check(mHandle.mGraphicsQueue.submit2(1, &submit, frame.mRenderFence));

  vk::PresentInfoKHR presentInfo{.waitSemaphoreCount = 1,
                                 .pWaitSemaphores = &swapchainEntry.semaphore,
                                 .swapchainCount = 1,
                                 .pSwapchains = &mHandle.mSwapchain,
                                 .pImageIndices = &swapchainImageIndex};
  // Present the image once render is complete
  check(mHandle.mGraphicsQueue.presentKHR(&presentInfo));
  mFrameNumber++;
}

void VulkanEngine::shutdown() {
  assert(sEngineInstance == this && "Engine must exist to be shut down");

  fmt::println("Vulcanite shutting down. Goodbye!");

  // Let the GPU finish its work
  vkDeviceWaitIdle(mHandle.mDevice);
  for (auto &frameData : mFrameData) {
    frameData.destroy(mHandle);
  }
  mImgui.destroy(mHandle);

  mGradientShader.free();
  mGlobalDescriptorAllocator.destroy();
  // This will also destroy all descriptor sets allocated by it
  vkDestroyDescriptorSetLayout(mHandle.mDevice, mDrawImageDescriptorLayout,
                               nullptr);

  mDrawImage.destroy(mHandle);

  mHandle.shutdown();
  SDL_DestroyWindow(mWindow);

  sEngineInstance = nullptr;
}

} // namespace selwonk::vulkan
