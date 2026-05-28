#include "samplermanager.hpp"
#include "shader.hpp"
#include "utility.hpp"
#include "vulkan/vulkan.hpp"
#include "vulkanhandle.hpp"
#include <stdexcept>

namespace selwonk::vulkan {
vk::Filter
SamplerManager::convertFilter(fastgltf::Optional<fastgltf::Filter> filter) {
  using enum fastgltf::Filter;
  switch (filter.value_or(Nearest)) {
  case Nearest:
  case NearestMipMapLinear:
  case NearestMipMapNearest:
    return vk::Filter::eNearest;
  case Linear:
  case LinearMipMapLinear:
  case LinearMipMapNearest:
    return vk::Filter::eLinear;
  }
  std::unreachable();
}

vk::SamplerMipmapMode
SamplerManager::convertMipmapMode(fastgltf::Optional<fastgltf::Filter> mode) {
  using enum fastgltf::Filter;
  switch (mode.value_or(Nearest)) {
  case Nearest:
  case NearestMipMapLinear:
  case NearestMipMapNearest:
    return vk::SamplerMipmapMode::eNearest;
  case Linear:
  case LinearMipMapLinear:
  case LinearMipMapNearest:
    return vk::SamplerMipmapMode::eLinear;
  }
  std::unreachable();
}

SamplerManager::SamplerManager() {
  std::array<DescriptorAllocator::PoolSizeRatio, 1> sizes = {
      {{vk::DescriptorType::eSampler, 1}}};
  mAllocator.init(MaxSamplers, sizes);

  DescriptorLayoutBuilder builder;
  builder.addBinding(0, vk::DescriptorType::eSampler, MaxSamplers);
  mLayout = builder.build(VulkanHandle::get().mDevice,
                          vk::ShaderStageFlagBits::eFragment);
  mDescriptorSet = mAllocator.allocate(mLayout);
}

SamplerManager::~SamplerManager() {
  auto& handle = VulkanHandle::get();
  for (int i = 0; i < mNextSlot; i++) {
    handle.mDevice.destroySampler(mEntries[i].sampler, nullptr);
  }
  handle.mDevice.destroyDescriptorSetLayout(mLayout, nullptr);
  mAllocator.destroy();
}

SamplerManager::Handle SamplerManager::get(assets::SamplerConfig key) {
  auto sampler = find(key);
  if (sampler.valid())
    return sampler;

  if (mNextSlot >= MaxSamplers)
    throw std::runtime_error("Too many samplers");

  auto index = mNextSlot;
  mNextSlot++;

  auto& handle = VulkanHandle::get();
  mEntries[index].key = key;
  vk::SamplerCreateInfo info = {
      .magFilter = convertFilter(key.mMagFilter),
      .minFilter = convertFilter(key.mMinFilter),
      .mipmapMode = convertMipmapMode(key.mMinFilter),
      .minLod = 0,
      .maxLod = vk::LodClampNone,
  };

  CHECK(handle.mDevice.createSampler(&info, nullptr, &mEntries[index].sampler));

  DescriptorAllocator::writeSampler(mDescriptorSet, mEntries[index].sampler,
                                    index);

  return Handle(index, 0);
}

SamplerManager::Handle SamplerManager::find(assets::SamplerConfig key) {
  for (int i = 0; i < mNextSlot; i++) {
    if (mEntries[i].key == key) {
      return Handle(i, 0);
    }
  }
  return Handle();
}

} // namespace selwonk::vulkan
