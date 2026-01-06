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
  mDescriptorSet = mAllocator.allocate<SamplerDescriptor>(mSamplerLayout);
}

SamplerCache::~SamplerCache() {
  auto& handle = VulkanHandle::get();
  for (auto& s : mData) {
    handle.mDevice.destroySampler(s, nullptr);
  }
  handle.mDevice.destroyDescriptorSetLayout(mSamplerLayout, nullptr);
  mAllocator.destroy();
}

vk::Sampler SamplerCache::create(const vk::SamplerCreateInfo& params,
                                 Handle index) {
  if (index.value() >= MaxSamplers)
    throw std::runtime_error("Too many samplers");

  vk::Sampler sampler;
  check(VulkanHandle::get().mDevice.createSampler(&params, nullptr, &sampler));
  updateSet(sampler, index);
  return sampler;
}

void SamplerCache::updateSet(vk::Sampler sampler, Handle index) {
  if (!mZeroed) {
    mZeroed = true;
    // Zero all slots as required for Vulcan to not complain
    for (int i = 0; i < MaxSamplers; i++) {
      updateSet(sampler, Handle(i));
    }
  }

  mDescriptorSet.write(VulkanHandle::get().mDevice, {sampler, index.value()});
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
