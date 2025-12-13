#include "vulkanengine.hpp"

#include <SDL3/SDL_init.h>
#include <SDL3/SDL_video.h>
#include <cassert>

#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>
#include <VkBootstrap.h>

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

  mSettings = settings;

  SDL_Init(SDL_INIT_VIDEO);
  mWindow = SDL_CreateWindow("Vulkan Engine", mSettings.size.x,
                             mSettings.size.y, SDL_WINDOW_VULKAN);

  mHandle.init(mSettings.mVulkan, mWindow);
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
  }
}

void VulkanEngine::shutdown() {
  assert(sEngineInstance == this && "Engine must exist to be shut down");
  sEngineInstance = nullptr;

  mHandle.shutdown();
  SDL_DestroyWindow(mWindow);
}

} // namespace selwonk::vk
