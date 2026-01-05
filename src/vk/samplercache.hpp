#pragma once

#include "vulkan/vulkan.hpp"

namespace selwonk::vulkan {
class SamplerCache {
public:
  const static constexpr size_t MaxSamplers = VN_MAXSAMPLERS;

  ~SamplerCache();
  vk::Sampler get(const vk::SamplerCreateInfo& params);

private:
  struct Sampler {
    vk::SamplerCreateInfo info;
    vk::Sampler sampler;
  };
  std::array<Sampler, MaxSamplers> mSamplers;

  std::optional<size_t> find(const vk::SamplerCreateInfo& info);

  size_t mNextIndex = 0;
};
} // namespace selwonk::vulkan
