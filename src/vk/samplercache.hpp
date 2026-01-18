#pragma once

#include <vulkan/vulkan.hpp>

#include "../core/cvar.hpp"
#include "resourcemap.hpp"
#include "shader.hpp"

namespace selwonk::vulkan {
struct CmpSamplerInfo {
  bool operator()(const vk::SamplerCreateInfo& lhs,
                  const vk::SamplerCreateInfo& rhs) const;
};

class SamplerCache : public ResourceMap<SamplerCache, vk::SamplerCreateInfo,
                                        vk::Sampler, CmpSamplerInfo> {
public:
  SamplerCache(core::Cvar::Int& maxSamplers);
  ~SamplerCache();

  vk::DescriptorSetLayout getDescriptorLayout() { return mSamplerLayout; }
  vk::DescriptorSet getDescriptorSet() { return mDescriptorSet.getSet(); }

  vk::Sampler create(const vk::SamplerCreateInfo& params, Handle index);
  int getCapacity() const { return mCapacity; }

private:
  void resize(int capacity);
  void updateSet(vk::Sampler sampler, Handle index);

  int mCapacity;
  DescriptorAllocator mAllocator;
  vk::DescriptorSetLayout mSamplerLayout;
  DescriptorSet<SamplerDescriptor> mDescriptorSet;
  bool mZeroed = false;
};
} // namespace selwonk::vulkan
