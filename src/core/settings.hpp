#pragma once

#include <glm/vec2.hpp>

namespace selwonk::core {
struct Settings {
  // Initial window size
  glm::uvec2 initialSize = glm::ivec2(1280, 720);
  // Request vulkan validation layers, performs extra checks on drawcalls
  // but adds a non-trivial overhead
  bool requestValidationLayers = true;
};
} // namespace selwonk::core
