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

  // TODO: Move this to assets along with GLTF loading
  // May not want to use fastgltf enums
  struct Key {
    fastgltf::Filter mMinFilter;
    fastgltf::Filter mMagFilter;

    constexpr bool operator==(const Key& other) const {
      return mMagFilter == other.mMinFilter && mMagFilter == other.mMagFilter;
    }
  };

  const static constexpr size_t MaxSamplers = 8;
  // TODO: Maybe remove this alias
  using Handle = assets::SamplerConfig::Handle;

  SamplerManager();
  ~SamplerManager();

  Handle get(Key key);
  vk::Sampler getSampler(Handle handle) {
    return mEntries[handle.value()].sampler;
  }

  vk::DescriptorSetLayout getDescriptorLayout() { return mLayout; }
  vk::DescriptorSet getDescriptorSet() { return mDescriptorSet; }

  int size() { return mNextSlot; }
  int capacity() { return MaxSamplers; }

private:
  Handle find(Key key);

  struct Entry {
    Key key;
    vk::Sampler sampler;
  };

  std::array<Entry, MaxSamplers> mEntries;

  DescriptorAllocator mAllocator;
  vk::DescriptorSetLayout mLayout;
  vk::DescriptorSet mDescriptorSet;
  int mNextSlot = 0;
};
} // namespace selwonk::vulkan
