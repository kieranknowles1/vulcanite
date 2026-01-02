#pragma once

#include "shader.hpp"

namespace selwonk::vulkan {
struct Material {
  enum class Pass : uint8_t {
    // Opaque or alpha-tested
    Opaque,
    // Translucent via alpha channel
    Translucent,
  };

  Pipeline* mPipeline;
  DescriptorSet<ImageSamplerDescriptor> mTexture;
  Pass mPass;
};
} // namespace selwonk::vulkan
