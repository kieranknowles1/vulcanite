#pragma once

#include "fastgltf/types.hpp"
#include "shader.hpp"
#include "vnassets/sampler.hpp"
#include "vulkan/vulkan.hpp"

namespace selwonk::vulkan {
// Manager for all sampler objects. Samplers are never destroyed after
// creation. Therefore ref counting is not applicable
class SamplerManager {
public:
  static vk::Filter convertFilter(fastgltf::Optional<fastgltf::Filter> filter);
  static vk::SamplerMipmapMode
  convertMipmapMode(fastgltf::Optional<fastgltf::Filter> mode);

  const static constexpr size_t MaxSamplers = 8;
  // TODO: Maybe remove this alias
  using Handle = assets::SamplerConfig::Handle;

  SamplerManager();
  ~SamplerManager();

  Handle get(assets::SamplerConfig key);
  vk::Sampler getSampler(Handle handle) {
    return mEntries[handle.value()].sampler;
  }

  vk::DescriptorSetLayout getDescriptorLayout() { return mLayout; }
  vk::DescriptorSet getDescriptorSet() { return mDescriptorSet; }

  int size() { return mNextSlot; }
  int capacity() { return MaxSamplers; }

private:
  Handle find(assets::SamplerConfig key);

  struct Entry {
    assets::SamplerConfig key;
    vk::Sampler sampler;
  };

  std::array<Entry, MaxSamplers> mEntries;

  DescriptorAllocator mAllocator;
  vk::DescriptorSetLayout mLayout;
  vk::DescriptorSet mDescriptorSet;
  int mNextSlot = 0;
};
} // namespace selwonk::vulkan
