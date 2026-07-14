#pragma once

#include <vncore/singleton.hpp>
#include <vncore/vfs.hpp>

#include "sampler.hpp"
#include "image.hpp"
#include "material.hpp"

namespace selwonk::assets {

// Main interface for providing native handles to higher-level components and interacting with them
class INativeHandleProvider : public core::Singleton<INativeHandleProvider> {
public:
#pragma region Samplers
  virtual SamplerConfig::Handle getSampler(SamplerConfig definition) = 0;
#pragma endregion

#pragma region Textures
  virtual ImageBase::Handle loadTextureAsync(
    const char* name,
    const fastgltf::Asset& asset,
    const fastgltf::DataSource& data
  ) = 0;
  virtual ImageBase::Handle loadTextureFromFileAsync(
    const char* name,
    core::Vfs::Path path
  ) = 0;

  virtual ImageBase::Handle getWhite() = 0;

  virtual void incRef(ImageBase::Handle handle) = 0;
  virtual bool decRef(ImageBase::Handle handle) = 0;
#pragma endregion

#pragma region Materials
  virtual Material::DataHandle addMaterial(const interop::MaterialData& data) = 0;
  virtual void incRef(Material::DataHandle handle) = 0;
  virtual bool decRef(Material::DataHandle handle) = 0;
#pragma endregion
};

}