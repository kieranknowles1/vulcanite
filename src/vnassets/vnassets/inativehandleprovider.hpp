#pragma once

#include "sampler.hpp"

namespace selwonk::assets {

class INativeHandleProvider : public core::Singleton<INativeHandleProvider> {
public:
  virtual SamplerConfig::Handle getSampler(SamplerConfig definition) = 0;
};

};