#pragma once

#include <vulkan/vulkan.hpp>

#include "../core/resourcemap.hpp"
#include "shader.hpp"

namespace selwonk::vulkan {
struct CmpSamplerInfo {
  bool operator()(const vk::SamplerCreateInfo& lhs,
                  const vk::SamplerCreateInfo& rhs) const;
};

class SamplerCache : public ResourceMap<SamplerCache, vk::SamplerCreateInfo,
                                        vk::Sampler, CmpSamplerInfo> {
public:
  const static constexpr size_t MaxSamplers = VN_MAXSAMPLERS;

  SamplerCache();
  ~SamplerCache();

  vk::DescriptorSetLayout getDescriptorLayout() { return mSamplerLayout; }
  vk::DescriptorSet getDescriptorSet() { return mDescriptorSet.getSet(); }

  vk::Sampler create(const vk::SamplerCreateInfo& params, Handle index);

private:
  void updateSet(vk::Sampler sampler, Handle index);

  DescriptorAllocator mAllocator;
  vk::DescriptorSetLayout mSamplerLayout;
  DescriptorSet<SamplerDescriptor> mDescriptorSet;
  bool mZeroed = false;
};
} // namespace selwonk::vulkan
