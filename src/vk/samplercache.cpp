#include "samplercache.hpp"

#include "utility.hpp"
#include "vulkanhandle.hpp"

namespace selwonk::vulkan {

SamplerCache::~SamplerCache() {
  auto& handle = VulkanHandle::get();
  for (auto& s : mSamplers) {
    handle.mDevice.destroySampler(s.sampler, nullptr);
  }
}

vk::Sampler SamplerCache::get(const vk::SamplerCreateInfo& params) {
  auto index = find(params);
  if (index != std::nullopt) {
    assert(mSamplers[index.value()].info == params);
    return mSamplers[index.value()].sampler;
  }

  if (mNextIndex >= mSamplers.size())
    throw std::runtime_error("Too many samplers");

  vk::Sampler sampler;
  check(VulkanHandle::get().mDevice.createSampler(&params, nullptr, &sampler));
  mSamplers[mNextIndex] = {params, sampler};
  mNextIndex++;
  return sampler;
}

std::optional<size_t> SamplerCache::find(const vk::SamplerCreateInfo& info) {
  for (int i = 0; i < mNextIndex; i++) {
    if (mSamplers[i].info == info)
      return i;
  }
  return std::nullopt;
}

} // namespace selwonk::vulkan
