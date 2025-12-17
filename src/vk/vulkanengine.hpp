#pragma once

#include <array>

#include <SDL3/SDL_video.h>
#include <glm/vec2.hpp>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

#include "../core/window.hpp"
#include "../vfs.hpp"
#include "shader.hpp"
#include "vulkanhandle.hpp"

namespace selwonk::vk {
class VulkanEngine {
public:
  struct FrameData {
    VkCommandPool mCommandPool;     // Allocator for command buffers
    VkCommandBuffer mCommandBuffer; // Pool of commands yet to be submitted

    VkSemaphore
        mSwapchainSemaphore; // Tell the GPU when the GPU is done rendering
    VkFence mRenderFence;    // Tell the CPU when the GPU is done rendering

    void init(VulkanHandle &handle);
    void destroy(VulkanHandle &handle);
  };

  static constexpr unsigned int BufferCount = 2;

  static VulkanEngine &get();

  VulkanEngine(core::Window &window, VulkanHandle &handle);
  ~VulkanEngine();

  void run();

  FrameData &getCurrentFrame() {
    return mFrameData[mFrameNumber % BufferCount];
  }

  VulkanHandle &getVulkan() { return mHandle; }
  Vfs &getVfs() { return *mVfs; }

private:
  void initCommands();
  void initDescriptors();

  void draw();
  void drawBackground(VkCommandBuffer cmd);

  core::Window &mWindow;
  VulkanHandle &mHandle;

  Image mDrawImage;
  VkExtent2D mDrawExtent;

  DescriptorAllocator mGlobalDescriptorAllocator;
  VkDescriptorSet mDrawImageDescriptors;
  VkDescriptorSetLayout mDrawImageDescriptorLayout;

  Shader mGradientShader;

  std::array<FrameData, BufferCount> mFrameData;

  std::unique_ptr<Vfs> mVfs;

  unsigned int mFrameNumber = 0;
};
} // namespace selwonk::vk
