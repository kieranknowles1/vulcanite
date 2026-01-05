#pragma once

#include "shader.hpp"
#include "vulkan/vulkan.hpp"

namespace selwonk::vulkan {
class SamplerCache {
public:
  using SamplerId = uint8_t;
  const static constexpr SamplerId MaxSamplers = VN_MAXSAMPLERS;

  SamplerCache();
  ~SamplerCache();
  SamplerId get(const vk::SamplerCreateInfo& params);

  vk::DescriptorSetLayout getDescriptorLayout() { return mSamplerLayout; }
  vk::DescriptorSet getDescriptorSet() { return mDescriptorSet.getSet(); }

private:
  void updateSets(int usedCount);

  struct Sampler {
    vk::SamplerCreateInfo info;
    vk::Sampler sampler;
  };
  std::array<Sampler, MaxSamplers> mSamplers;

  std::optional<SamplerId> find(const vk::SamplerCreateInfo& info);
  DescriptorAllocator mAllocator;
  vk::DescriptorSetLayout mSamplerLayout;
  DescriptorSet<SamplerArrayDescriptor> mDescriptorSet;

  SamplerId mNextIndex = 0;
};
} // namespace selwonk::vulkan
