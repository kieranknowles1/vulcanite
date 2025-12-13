#include "utility.hpp"
#include "vulkanengine.hpp"
#include "vulkaninit.hpp"

#include <SDL3/SDL_init.h>
#include <SDL3/SDL_video.h>
#include <cassert>

#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>
#include <VkBootstrap.h>
#include <fmt/base.h>
#include <vulkan/vulkan_core.h>

namespace selwonk::vk {
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
  mWindow = SDL_CreateWindow("Vulkan Engine", mSettings.size.x,
                             mSettings.size.y, SDL_WINDOW_VULKAN);

  mHandle.init(mSettings.mVulkan, mSettings.size, mWindow);

  // No more VkBootstrap - you're on your own now.
  initCommands();

  fmt::println("Ready to go!");
}

void VulkanEngine::initCommands() {
  fmt::println("Initialising command buffers");
  auto poolInfo =
      VulkanInit::commandPoolCreateInfo(mHandle.mGraphicsQueueFamily);

  for (int i = 0; i < mFrameData.size(); i++) {
    // Allocate a pool that will allocate buffers
    check(vkCreateCommandPool(mHandle.mDevice, &poolInfo, nullptr,
                              &mFrameData[i].mCommandPool));

    // Allocate a default command buffer to submit into
    auto allocInfo = VulkanInit::bufferAllocateInfo(mFrameData[i].mCommandPool);
    check(vkAllocateCommandBuffers(mHandle.mDevice, &allocInfo,
                                   &mFrameData[i].mCommandBuffer));
  }
}

void VulkanEngine::run() {
  SDL_Event e;
  bool quit = false;
  int i = 0;

  while (!quit && i < 100) {
    while (SDL_PollEvent(&e)) {
      if (e.type == SDL_EVENT_QUIT) {
        quit = true;
      }
    }
    mFrameNumber++;
    i++;
  }
}

void VulkanEngine::shutdown() {
  assert(sEngineInstance == this && "Engine must exist to be shut down");
  sEngineInstance = nullptr;

  fmt::println("Vulcanite shutting down. Goodbye!");

  // Let the GPU finish its work
  vkDeviceWaitIdle(mHandle.mDevice);
  for (auto &frameData : mFrameData) {
    vkDestroyCommandPool(mHandle.mDevice, frameData.mCommandPool, nullptr);
    // Destroying a queue will destroy all its buffers
  }

  mHandle.shutdown();
  SDL_DestroyWindow(mWindow);
}

} // namespace selwonk::vk
