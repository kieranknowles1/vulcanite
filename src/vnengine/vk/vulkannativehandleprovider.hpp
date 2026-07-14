#pragma once

#include <vnassets/inativehandleprovider.hpp>

#include "samplermanager.hpp"
#include "texturemanager.hpp"

namespace selwonk::vulkan {

class VulkanNativeHandleProvider final : public assets::INativeHandleProvider {
public:
  static core::Cvar::Int MaxTextures;

  VulkanNativeHandleProvider();

  assets::SamplerConfig::Handle getSampler(assets::SamplerConfig definition) override {
    return mSamplers.get(definition);
  }
  assets::ImageBase::Handle loadTextureAsync(
    const char* name,
    const fastgltf::Asset& asset,
    const fastgltf::DataSource& data
  ) override {
    return mTextures.loadAsync(name, asset, data);
  }

  SamplerManager& getNativeSamplers() { return mSamplers; }
  TextureManager& getNativeTextures() { return mTextures; }
private:
  SamplerManager mSamplers;
  TextureManager mTextures;
};

}