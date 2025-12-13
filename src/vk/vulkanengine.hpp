#pragma once

#include <SDL3/SDL_video.h>
#include <glm/vec2.hpp>
#include <vulkan/vulkan.h>

#include "../definitions.hpp"
#include "vulkanhandle.hpp"

namespace selwonk::vk {
class VulkanEngine {
public:
  struct EngineSettings {
    glm::uvec2 size = glm::ivec2(1280, 720);
    VulkanHandle::Settings mVulkan;
  };
  static VulkanEngine &get();

  VulkanEngine();
  ~VulkanEngine();

  void init(EngineSettings settings);
  void run();
  void shutdown();

private:
  EngineSettings mSettings;

  SDL_Window *mWindow;
  VulkanHandle mHandle;

  /** Total time elapsed since start of engine, in seconds */
  Duration mGlobalTime = Duration::zero();
};
} // namespace selwonk::vk
