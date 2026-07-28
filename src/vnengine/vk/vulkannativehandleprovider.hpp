#pragma once

#include <vnassets/inativehandleprovider.hpp>

#include "bufferarray.hpp"
#include "samplermanager.hpp"
#include "texturemanager.hpp"
#include "vnassets/image.hpp"
#include "vnassets/material.hpp"

namespace selwonk::vulkan {

#define IMPL_REFS(HandleType, Container)                                       \
  void incRef(HandleType handle) override { return Container.incRef(handle); } \
  bool decRef(HandleType handle) override { return Container.decRef(handle); }

class VulkanNativeHandleProvider final : public assets::INativeHandleProvider {
public:
  static core::Cvar::Int MaxTextures;

  VulkanNativeHandleProvider();

#pragma region Samplers
  assets::SamplerConfig::Handle
  getSampler(assets::SamplerConfig definition) override {
    return mSamplers.get(definition);
  }

#pragma endregion

#pragma region Textures
  assets::ImageBase::Handle
  loadTextureAsync(const char* name, const fastgltf::Asset& asset,
                   const fastgltf::DataSource& data) override {
    return mTextures.loadAsync(name, asset, data);
  }
  assets::ImageBase::Handle
  loadTextureFromFileAsync(const char* name, core::Vfs::Path path) override {
    return mTextures.loadAsync(name, path);
  }

  assets::ImageBase::Handle getWhite() override { return mTextures.getWhite(); }
  IMPL_REFS(assets::ImageBase::Handle, mTextures);

#pragma endregion

#pragma region Materials
  assets::Material::DataHandle
  addMaterial(const interop::MaterialData& data) override {
    return mMaterials.insert(data);
  }
  IMPL_REFS(assets::Material::DataHandle, mMaterials);
#pragma endregion

  SamplerManager& getNativeSamplers() { return mSamplers; }
  TextureManager& getNativeTextures() { return mTextures; }
  BufferArray<interop::MaterialData>& getNativeMaterials() {
    return mMaterials;
  }

private:
  SamplerManager mSamplers;
  TextureManager mTextures;
  BufferArray<interop::MaterialData> mMaterials;
};

#undef IMPL_REFS

} // namespace selwonk::vulkan
