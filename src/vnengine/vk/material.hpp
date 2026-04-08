#pragma once

#include "../../assets/shaders/triangle.h"
#include "bufferarray.hpp"
#include "samplermanager.hpp"
#include "texturemanager.hpp"

namespace selwonk::vulkan {
struct Material {
  enum class Pass : uint8_t {
    // Opaque or alpha-tested
    Opaque,
    // Translucent via alpha channel
    Translucent,
  };

  // TODO: Do we need this struct or can meshes hold material data handles
  // directly
  TextureManager::Handle mTexture;
  BufferArray<interop::MaterialData>::Handle mDataIndex;
  SamplerManager::Handle mSampler;
  Pass mPass;
};
} // namespace selwonk::vulkan
