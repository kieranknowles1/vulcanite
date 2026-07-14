#pragma once

#include <vnassets/inativehandleprovider.hpp>

#include "bufferarray.hpp"
#include "samplermanager.hpp"
#include "texturemanager.hpp"

namespace selwonk::vulkan {

class VulkanNativeHandleProvider final : public assets::INativeHandleProvider {
public:
  static core::Cvar::Int MaxTextures;

  VulkanNativeHandleProvider();

#pragma region Samplers
  assets::SamplerConfig::Handle getSampler(assets::SamplerConfig definition) override {
    return mSamplers.get(definition);
  }

#pragma endregion

#pragma region Textures
  assets::ImageBase::Handle loadTextureAsync(
    const char* name,
    const fastgltf::Asset& asset,
    const fastgltf::DataSource& data
  ) override {
    return mTextures.loadAsync(name, asset, data);
  }
  assets::ImageBase::Handle loadTextureFromFileAsync(
    const char* name,
    core::Vfs::Path path
  ) override {
    return mTextures.loadAsync(name, path);
  }

  assets::ImageBase::Handle getWhite() override { return mTextures.getWhite(); }
  void incRef(assets::ImageBase::Handle handle) override { return mTextures.incRef(handle); }
  bool decRef(assets::ImageBase::Handle handle) override { return mTextures.decRef(handle); }

#pragma endregion

#pragma region Materials
  assets::Material::DataHandle addMaterial(const interop::MaterialData& data) override {
    return mMaterials.insert(data);
  }
  void incRef(assets::Material::DataHandle handle) override { return mMaterials.incRef(handle); }
  bool decRef(assets::Material::DataHandle handle) override { return mMaterials.decRef(handle); }
#pragma endregion

  SamplerManager& getNativeSamplers() { return mSamplers; }
  TextureManager& getNativeTextures() { return mTextures; }
  BufferArray<interop::MaterialData>& getNativeMaterials() { return mMaterials; }
private:
  SamplerManager mSamplers;
  TextureManager mTextures;
  BufferArray<interop::MaterialData> mMaterials;
};

}