#pragma once

#include "../../assets/shaders/interop.h"
#include "buffer.hpp"
#include "samplercache.hpp"
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
  DescriptorSet<ImageDescriptor> mTexture;
  Buffer::CrossAllocation<interop::MaterialData> mData;
  SamplerCache::Handle mSampler;
  Pass mPass;
};
} // namespace selwonk::vulkan
