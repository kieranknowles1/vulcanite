#pragma once

#include "../../assets/shaders/interop.h"
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
  interop::MaterialData* mData;
  Pass mPass;
};
} // namespace selwonk::vulkan
