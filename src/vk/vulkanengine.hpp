#pragma once

#include <SDL3/SDL_video.h>
#include <glm/vec2.hpp>

#include "../definitions.hpp"

namespace selwonk::vk {
class VulkanEngine {
public:
  struct WindowSettings {
    glm::uvec2 size = glm::ivec2(1280, 720);
  };
  static VulkanEngine& get();

  VulkanEngine();
  ~VulkanEngine();

  void init(WindowSettings settings);
  void run();
  void shutdown();

private:
  WindowSettings mSettings;
  SDL_Window* mWindow;
  /** Total time elapsed since start of engine, in seconds */
  Duration mGlobalTime = Duration::zero();
};
} // namespace selwonk::vk
