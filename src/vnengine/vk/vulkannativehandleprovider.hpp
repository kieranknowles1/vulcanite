#pragma once

#include <vnassets/inativehandleprovider.hpp>

#include "samplermanager.hpp"

namespace selwonk::vulkan {

class VulkanNativeHandleProvider final : public assets::INativeHandleProvider {
public:
  assets::SamplerConfig::Handle getSampler(assets::SamplerConfig definition) override {
    return mSamplers.get(definition);
  }

  SamplerManager& getNativeSamplers() { return mSamplers; }
private:
  SamplerManager mSamplers;
};

};