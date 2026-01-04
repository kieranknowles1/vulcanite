#pragma once

#include <glm/vec2.hpp>
#include <vulkan/vulkan_core.h>

namespace selwonk::core {
struct Settings {
  // Initial window size
  glm::uvec2 initialSize = glm::ivec2(1280, 720);
  // Request vulkan validation layers, performs extra checks on API calls but
  // adds significant overhead. Should be disabled when performance is required
  bool requestValidationLayers = true;

  enum class VsyncMode : uint8_t {
    // Present frames immediately, may cause tearing
    None = VK_PRESENT_MODE_IMMEDIATE_KHR,
    // Present frames on next vsync, replacing the pending frame if rendering
    // outpaces refresh
    LowLatency = VK_PRESENT_MODE_MAILBOX_KHR,
    // Present frames on next vsync, waiting for it if necessary. Your classic
    // vsync. Caps framerate
    Strict = VK_PRESENT_MODE_FIFO_KHR,
    // Present frames on next vsync, unless a vblank was missed causing the
    // pending frame to arrive late then push immediately. Caps framerate
    StutterFree = VK_PRESENT_MODE_FIFO_RELAXED_KHR,
  };
  VsyncMode vsync = VsyncMode::StutterFree;
};
} // namespace selwonk::core
