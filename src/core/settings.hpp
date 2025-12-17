#pragma once

#include <glm/ext/vector_uint2.hpp>
namespace selwonk::core {
struct Settings {
  static const Settings &get() {
    static Settings instance;
    return instance;
  }

  glm::uvec2 windowSize = glm::uvec2(1280, 720);
  // Add additional validation to Vulkan calls. Comes with non-trivial overhead.
  bool requestValidationLayers = true;
};
} // namespace selwonk::core
