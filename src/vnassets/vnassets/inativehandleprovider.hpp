#pragma once

#include <vncore/singleton.hpp>

#include "sampler.hpp"
#include "image.hpp"

namespace selwonk::assets {

class INativeHandleProvider : public core::Singleton<INativeHandleProvider> {
public:
  virtual SamplerConfig::Handle getSampler(SamplerConfig definition) = 0;

  virtual ImageBase::Handle loadTextureAsync(
    const char* name,
    const fastgltf::Asset& asset,
    const fastgltf::DataSource& data
  ) = 0;
};

}