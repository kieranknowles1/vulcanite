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
  for (auto& s : mData) {
    handle.mDevice.destroySampler(s, nullptr);
  }
  handle.mDevice.destroyDescriptorSetLayout(mSamplerLayout, nullptr);
  mAllocator.destroy();
}

vk::Sampler SamplerCache::create(const vk::SamplerCreateInfo& params) {
  if (mData.size() >= MaxSamplers)
    throw std::runtime_error("Too many samplers");

  vk::Sampler sampler;
  check(VulkanHandle::get().mDevice.createSampler(&params, nullptr, &sampler));
  return sampler;
}

void SamplerCache::updateSets() {
  SamplerArrayDescriptor data{.mData = mData};
  // We need to write the whole sampler array at least once, otherwise the
  // validation layers will complain
  for (int i = mData.size(); i < MaxSamplers; i++) {
    data.mData.emplace_back(mData[0]);
  }
  mDescriptorSet.write(VulkanHandle::get().mDevice, data);
}

bool CmpSamplerInfo::operator()(const vk::SamplerCreateInfo& lhs,
                                const vk::SamplerCreateInfo& rhs) const {

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
