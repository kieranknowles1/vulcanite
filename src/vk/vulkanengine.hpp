#pragma once

#include <array>

#include <SDL3/SDL_video.h>
#include <glm/vec2.hpp>
#include <vulkan/vulkan.h>

#include "vulkanhandle.hpp"

namespace selwonk::vk {
class VulkanEngine {
public:
  struct EngineSettings {
    glm::uvec2 size = glm::ivec2(1280, 720);
    VulkanHandle::Settings mVulkan;
  };

  struct FrameData {
    VkCommandPool mCommandPool;     // Allocator for command buffers
    VkCommandBuffer mCommandBuffer; // Pool of commands yet to be submitted
  };

  static constexpr unsigned int BufferCount = 2;

  static VulkanEngine &get();

  VulkanEngine();
  ~VulkanEngine();

  void init(EngineSettings settings);
  void run();
  void shutdown();

  FrameData &getCurrentFrame() {
    return mFrameData[mFrameNumber % BufferCount];
  }

private:
  void initCommands();

  EngineSettings mSettings;

  SDL_Window *mWindow;
  VulkanHandle mHandle;

  std::array<FrameData, BufferCount> mFrameData;

  unsigned int mFrameNumber = 0;
};
} // namespace selwonk::vk
