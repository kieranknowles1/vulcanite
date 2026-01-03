#include "samplercache.hpp"

#include "utility.hpp"
#include "vulkanhandle.hpp"

namespace selwonk::vulkan {

SamplerCache::~SamplerCache() {
  auto& handle = VulkanHandle::get();
  for (auto& s : mSamplers) {
    handle.mDevice.destroySampler(s.second, nullptr);
  }
}

vk::Sampler SamplerCache::get(const vk::SamplerCreateInfo& params) {
  auto it = mSamplers.find(params);
  if (it != mSamplers.end()) {
    assert(it->first == params);
    return it->second;
  }

  vk::Sampler sampler;
  check(VulkanHandle::get().mDevice.createSampler(&params, nullptr, &sampler));
  mSamplers.insert(std::make_pair(params, sampler));
  return sampler;
}

bool SamplerCache::CmpSamplerInfo::operator()(
    const vk::SamplerCreateInfo& lhs, const vk::SamplerCreateInfo& rhs) const {
#define CMP(field)                                                             \
  if (lhs.field != rhs.field)                                                  \
    return lhs.field < rhs.field;

  CMP(flags);
  CMP(magFilter);
  CMP(minFilter);
  CMP(mipmapMode);
  CMP(addressModeU);
  CMP(addressModeV);
  CMP(addressModeW);
  CMP(mipLodBias);
  CMP(anisotropyEnable);
  CMP(maxAnisotropy);
  CMP(compareEnable);
  CMP(compareOp);
  CMP(minLod);
  CMP(maxLod);
  CMP(borderColor);
  CMP(unnormalizedCoordinates);
  return false;

#undef CMP
}

} // namespace selwonk::vulkan
