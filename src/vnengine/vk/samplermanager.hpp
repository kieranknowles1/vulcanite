#pragma once

#include "vncore/handle.hpp"
#include "vulkan/vulkan.hpp"
#include <vnvulkan/shader.hpp>

namespace selwonk::vulkan {
// Manager for all sampler objects. Samplers are never destroyed after
// creation. Therefore ref counting is not applicable
class SamplerManager {
public:
  const static constexpr size_t MaxSamplers = 32;
  using Handle = core::Handle<SamplerManager>;

  SamplerManager();
  ~SamplerManager();

  Handle get(vk::SamplerCreateInfo info);
  vk::Sampler getSampler(Handle handle) {
    return mEntries[handle.value()].sampler;
  }

  vk::DescriptorSetLayout getDescriptorLayout() { return mLayout; }
  vk::DescriptorSet getDescriptorSet() { return mDescriptorSet; }

  int size() { return mNextSlot; }
  int capacity() { return MaxSamplers; }

private:
  Handle find(vk::SamplerCreateInfo info);

  struct Entry {
    vk::SamplerCreateInfo info;
    vk::Sampler sampler;
  };

  std::array<Entry, MaxSamplers> mEntries;

  DescriptorAllocator mAllocator;
  vk::DescriptorSetLayout mLayout;
  vk::DescriptorSet mDescriptorSet;
  int mNextSlot = 0;
};
} // namespace selwonk::vulkan
