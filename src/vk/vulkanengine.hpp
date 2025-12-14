#pragma once

#include <array>

#include <SDL3/SDL_video.h>
#include <glm/vec2.hpp>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

#include "vulkanhandle.hpp"
#include "window.hpp"

namespace selwonk::vk {
class VulkanEngine {
public:
  struct EngineSettings {
    Window::Settings mWindow;
    VulkanHandle::Settings mVulkan;
  };

  struct FrameData {
    VkCommandPool mCommandPool;     // Allocator for command buffers
    VkCommandBuffer mCommandBuffer; // Pool of commands yet to be submitted

    VkSemaphore mSwapchainSemaphore;
    VkSemaphore mRenderSemaphore; // Tell the GPU when the GPU is done rendering
    VkFence mRenderFence;         // Tell the CPU when the GPU is done rendering

    void init(VulkanHandle &handle);
    void destroy(VulkanHandle &handle);
  };

  static constexpr unsigned int BufferCount = 2;

  static VulkanEngine &get();

  VulkanEngine(Window &window, VulkanHandle &vulkan, EngineSettings settings);
  ~VulkanEngine();

  void run();
  void shutdown();

  FrameData &getCurrentFrame() {
    return mFrameData[mFrameNumber % BufferCount];
  }

  VulkanHandle &getVulkan() { return mHandle; }

private:
  void initCommands();
  void draw();

  EngineSettings mSettings;

  Window &mWindow;
  VulkanHandle &mHandle;

  std::array<FrameData, BufferCount> mFrameData;

  unsigned int mFrameNumber = 0;
};
} // namespace selwonk::vk
