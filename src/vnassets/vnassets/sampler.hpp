#pragma once

#include "fastgltf/types.hpp"
#include <vncore/handle.hpp>

namespace selwonk::assets {
struct SamplerConfig {
  using Handle = core::Handle<SamplerConfig>;

  fastgltf::Filter mMinFilter;
  fastgltf::Filter mMagFilter;

  constexpr bool operator==(const SamplerConfig& other) const {
    return mMinFilter == other.mMinFilter && mMagFilter == other.mMagFilter;
  }

  // TODO: Move sampler params here
};
} // namespace selwonk::assets
