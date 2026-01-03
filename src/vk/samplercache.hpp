#pragma once

#include "vulkan/vulkan.hpp"
#include <map>

namespace selwonk::vulkan {
class SamplerCache {
public:
  ~SamplerCache();
  vk::Sampler get(const vk::SamplerCreateInfo& params);

private:
  struct CmpSamplerInfo {
    bool operator()(const vk::SamplerCreateInfo& lhs,
                    const vk::SamplerCreateInfo& rhs) const;
  };

  std::map<vk::SamplerCreateInfo, vk::Sampler, CmpSamplerInfo> mSamplers;
};
} // namespace selwonk::vulkan
