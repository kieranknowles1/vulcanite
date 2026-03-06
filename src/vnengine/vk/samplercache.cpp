#include "samplercache.hpp"

#include "shader.hpp"
#include "utility.hpp"
#include "vulkan/vulkan.hpp"
#include "vulkanhandle.hpp"
#include <array>
#include <fmt/base.h>

namespace selwonk::vulkan {

SamplerCache::SamplerCache(core::Cvar::Int& maxSamplers)
    : mCapacity(maxSamplers.value()) {
  resize(mCapacity);
  maxSamplers.addChangeCallback([this](int capacity) { resize(capacity); });
  maxSamplers.addValidationCallback(
      [this](int capacity) -> std::optional<std::string> {
        if (capacity < mData.size()) {
          return "Cannot be lower than allocated samplers (" +
                 std::to_string(mData.size()) + ")";
        }
        return std::nullopt;
      });
}

void SamplerCache::resize(int capacity) {
  mCapacity = capacity;
  // TODO: Free layout/allocator once it's no longer used (multiple frames can
  // be in flight)

  std::array<DescriptorAllocator::PoolSizeRatio, 1> sizes = {
      {{vk::DescriptorType::eSampler, 1}}};
  mAllocator.init(capacity, sizes);

  DescriptorLayoutBuilder builder;
  builder.addBinding(0, vk::DescriptorType::eSampler, capacity);
  mSamplerLayout = builder.build(VulkanHandle::get().mDevice,
                                 vk::ShaderStageFlagBits::eFragment);
  mDescriptorSet = mAllocator.allocate<SamplerDescriptor>(mSamplerLayout);

  // TODO: Is there a better way than zeroing manually
  if (mZeroed) {
    updateSet(mData[0], Handle(0));
  }
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
  if (index.value() >= mCapacity)
    throw std::runtime_error("Too many samplers");

  vk::Sampler sampler;
  check(VulkanHandle::get().mDevice.createSampler(&params, nullptr, &sampler));
  updateSet(sampler, index);
  return sampler;
}

void SamplerCache::updateSet(vk::Sampler sampler, Handle index) {
  if (!mZeroed) {
    mZeroed = true;
    // Zero all slots as required for Vulkan to not complain
    for (int i = 0; i < mCapacity; i++) {
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
