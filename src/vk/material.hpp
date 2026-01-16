#pragma once

#include "samplercache.hpp"
#include "shader.hpp"
#include "texturemanager.hpp"

namespace selwonk::vulkan {
struct Material {
  enum class Pass : uint8_t {
    // Opaque or alpha-tested
    Opaque,
    // Translucent via alpha channel
    Translucent,
  };

  Pipeline* mPipeline;
  TextureManager::Handle mTexture;
  vk::DeviceAddress mData;
  SamplerCache::Handle mSampler;
  Pass mPass;
};
} // namespace selwonk::vulkan
