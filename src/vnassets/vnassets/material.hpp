#pragma once

#include "image.hpp"
#include "sampler.hpp"

#include "../../assets/shaders/triangle.h"

namespace selwonk::assets {
struct Material {
  using DataHandle = core::Handle<interop::MaterialData>;

  enum class Pass : uint8_t {
    // Opaque or alpha-tested
    Opaque,
    // Translucent via alpha channel
    Translucent,
  };

  // TODO: Do we need this struct or should a mesh's surface hold it directly
  ImageBase::Handle mTexture;
  DataHandle mDataIndex;
  SamplerConfig::Handle mSampler;
  Pass mPass;
};
} // namespace selwonk::assets
