#include "samplercache.hpp"

#include "shader.hpp"
#include "utility.hpp"
#include "vulkan/vulkan.hpp"
#include "vulkanhandle.hpp"
#include <array>
#include <fmt/base.h>

namespace selwonk::vulkan {

SamplerCache::SamplerCache() {
  std::array<DescriptorAllocator::PoolSizeRatio, 1> sizes = {
      {{vk::DescriptorType::eSampler, 1}}};
  mAllocator.init(MaxSamplers, sizes);

  DescriptorLayoutBuilder builder;
  builder.addBinding(0, vk::DescriptorType::eSampler, MaxSamplers);
  mSamplerLayout = builder.build(VulkanHandle::get().mDevice,
                                 vk::ShaderStageFlagBits::eFragment);
  mDescriptorSet = mAllocator.allocate<SamplerArrayDescriptor>(mSamplerLayout);
}

SamplerCache::~SamplerCache() {
  auto& handle = VulkanHandle::get();
  for (auto& s : mSamplers) {
    handle.mDevice.destroySampler(s.sampler, nullptr);
  }
  handle.mDevice.destroyDescriptorSetLayout(mSamplerLayout, nullptr);
  mAllocator.destroy();
}

SamplerCache::SamplerId SamplerCache::get(const vk::SamplerCreateInfo& params) {
  auto index = find(params);
  if (index != std::nullopt) {
    assert(mSamplers[index.value()].info == params);
    return index.value();
  }

  if (mNextIndex >= mSamplers.size())
    throw std::runtime_error("Too many samplers");

  vk::Sampler sampler;
  check(VulkanHandle::get().mDevice.createSampler(&params, nullptr, &sampler));
  SamplerId alloc = mNextIndex;
  fmt::println("Allocate sampler {}", alloc);

  mSamplers[mNextIndex] = {params, sampler};
  mNextIndex++;
  updateSets(alloc + 1);
  return alloc;
}

void SamplerCache::updateSets(int usedCount) {
  SamplerArrayDescriptor data;
  data.mData.reserve(mSamplers.size());
  for (int i = 0; i < usedCount; i++) {
    data.mData.emplace_back(mSamplers[i].sampler);
  }
  // We need to write the whole sampler array at least once, otherwise the
  // validation layers will complain
  for (int i = usedCount; i < mSamplers.size(); i++) {
    data.mData.emplace_back(mSamplers[0].sampler);
  }
  mDescriptorSet.write(VulkanHandle::get().mDevice, data);
}

std::optional<SamplerCache::SamplerId>
SamplerCache::find(const vk::SamplerCreateInfo& info) {
  for (int i = 0; i < mNextIndex; i++) {
    if (mSamplers[i].info == info)
      return i;
  }
  return std::nullopt;
}

} // namespace selwonk::vulkan
