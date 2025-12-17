#include "../times.hpp"
#include "imagehelpers.hpp"
#include "shader.hpp"
#include "utility.hpp"
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
#include <memory>
#include <vulkan/vulkan_core.h>

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

  // Allocate an image to fill the window
  VkImageUsageFlags drawImageUsage =
      VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT |
      VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

  mDrawImage.init(mHandle, {mSettings.size.x, mSettings.size.y, 1},
                  VK_FORMAT_R16G16B16A16_SFLOAT, drawImageUsage);
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
  check(vkCreateCommandPool(handle.mDevice, &poolInfo, nullptr, &mCommandPool));

  // Allocate a default command buffer to submit into
  auto allocInfo = VulkanInit::bufferAllocateInfo(mCommandPool);
  check(vkAllocateCommandBuffers(handle.mDevice, &allocInfo, &mCommandBuffer));

  mSwapchainSemaphore = handle.createSemaphore();

  // Create the fence in the "signalled" state so we can wait on it immediately
  // Simplifies first-frame logic
  auto fenceInfo = VulkanInit::fenceCreateInfo(VK_FENCE_CREATE_SIGNALED_BIT);
  check(vkCreateFence(handle.mDevice, &fenceInfo, nullptr, &mRenderFence));
}

void VulkanEngine::FrameData::destroy(VulkanHandle &handle) {
  // Destroying a queue will destroy all its buffers
  vkDestroyCommandPool(handle.mDevice, mCommandPool, nullptr);
  handle.destroySemaphore(mSwapchainSemaphore);
  vkDestroyFence(handle.mDevice, mRenderFence, nullptr);
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
      {VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1}};

  // Reserve space for 10 such descriptors
  mGlobalDescriptorAllocator.init(10, sizes);

  // Allocate one of these descriptors
  DescriptorLayoutBuilder builder;
  builder.addBinding(0, sizes[0].type);
  mDrawImageDescriptorLayout =
      builder.build(mHandle.mDevice, VK_SHADER_STAGE_COMPUTE_BIT);
  mDrawImageDescriptors =
      mGlobalDescriptorAllocator.allocate(mDrawImageDescriptorLayout);

  // Point the descriptor to the draw image
  VkDescriptorImageInfo info = {
      .imageView = mDrawImage.getView(),
      .imageLayout = VK_IMAGE_LAYOUT_GENERAL,
  };

  // Write to the pool
  VkWriteDescriptorSet write = {
      .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
      .dstSet = mDrawImageDescriptors,
      .dstBinding = 0,
      .descriptorCount = 1,
      .descriptorType = sizes[0].type,
      .pImageInfo = &info,
  };
  vkUpdateDescriptorSets(mHandle.mDevice, 1, &write, 0, nullptr);

  ShaderStage stage("gradient.comp.spv", VK_SHADER_STAGE_COMPUTE_BIT, "main");
  mGradientShader.link(mDrawImageDescriptorLayout, stage);
}

void VulkanEngine::run() {
  SDL_Event e;
  bool quit = false;

  while (!quit) {
    while (SDL_PollEvent(&e)) {
      if (e.type == SDL_EVENT_QUIT) {
        quit = true;
      }
    }
    draw();
  }
}

void VulkanEngine::drawBackground(VkCommandBuffer cmd) {
  vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_COMPUTE,
                    mGradientShader.mPipeline);
  vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_COMPUTE,
                          mGradientShader.mLayout, /*firstSet=*/0,
                          /*descriptorSetCount=*/1, &mDrawImageDescriptors,
                          /*dynamicOffsetCount=*/0,
                          /*pDynamicOffsets=*/nullptr);
  const int workgroupSize = 16;
  vkCmdDispatch(cmd, std::ceil(mDrawExtent.width / workgroupSize),
                std::ceil(mDrawExtent.height / workgroupSize), 1);
}

void VulkanEngine::draw() {
  auto &frame = getCurrentFrame();
  auto timeout = millisToNanoSeconds(1000);

  // Wait for the previous frame to finish
  check(
      vkWaitForFences(mHandle.mDevice, 1, &frame.mRenderFence, true, timeout));
  check(vkResetFences(mHandle.mDevice, 1, &frame.mRenderFence));

  // Request a buffer to draw to
  uint32_t swapchainImageIndex;
  check(vkAcquireNextImageKHR(mHandle.mDevice, mHandle.mSwapchain, timeout,
                              frame.mSwapchainSemaphore, nullptr,
                              &swapchainImageIndex));
  auto &swapchainImage = mHandle.mSwapchainImages[swapchainImageIndex];
  auto &renderSemaphore = mHandle.mRenderSemaphores[swapchainImageIndex];

  auto cmd = frame.mCommandBuffer;
  // We're certain the command buffer is not in use, prepare for recording
  check(vkResetCommandBuffer(cmd, 0));
  // We won't be submitting the buffer multiple times in a row, let Vulkan know
  // Drivers may be able to get a small speed boost
  auto beginInfo = VulkanInit::commandBufferBeginInfo(
      VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
  check(vkBeginCommandBuffer(cmd, &beginInfo));

  // Make the draw image writable, we don't care about destroying previous
  // data
  ImageHelpers::transitionImage(cmd, mDrawImage.getImage(),
                                VK_IMAGE_LAYOUT_UNDEFINED,
                                VK_IMAGE_LAYOUT_GENERAL);

  drawBackground(cmd);

  // Make the draw image readable again
  ImageHelpers::transitionImage(cmd, mDrawImage.getImage(),
                                VK_IMAGE_LAYOUT_GENERAL,
                                VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);

  // Copy draw image to the swapchain
  ImageHelpers::transitionImage(cmd, swapchainImage, VK_IMAGE_LAYOUT_UNDEFINED,
                                VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
  Image::copyToSwapchainImage(cmd, mDrawImage, swapchainImage,
                              mHandle.mSwapchainExtent);
  ImageHelpers::transitionImage(cmd, swapchainImage,
                                VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);

  // Finalise the command buffer, ready for execution
  check(vkEndCommandBuffer(cmd));

  // Submit, after all this time
  auto cmdInfo = VulkanInit::commandBufferSubmitInfo(cmd);
  auto waitInfo = VulkanInit::semaphoreSubmitInfo(
      frame.mSwapchainSemaphore,
      VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT_KHR);
  auto signalInfo = VulkanInit::semaphoreSubmitInfo(
      renderSemaphore, VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT);
  auto submit = VulkanInit::submitInfo(&cmdInfo, &waitInfo, &signalInfo);
  // Execute
  check(vkQueueSubmit2(mHandle.mGraphicsQueue, 1, &submit, frame.mRenderFence));

  // Present the image once render is complete
  VkPresentInfoKHR presentInfo = {.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
                                  .pNext = nullptr,
                                  .waitSemaphoreCount = 1,
                                  .pWaitSemaphores = &renderSemaphore,
                                  .swapchainCount = 1,
                                  .pSwapchains = &mHandle.mSwapchain,
                                  .pImageIndices = &swapchainImageIndex};
  check(vkQueuePresentKHR(mHandle.mGraphicsQueue, &presentInfo));
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
