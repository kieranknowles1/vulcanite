#include "samplermanager.hpp"
#include "shader.hpp"
#include "utility.hpp"
#include "vulkan/vulkan.hpp"
#include "vulkanhandle.hpp"
#include <stdexcept>

namespace selwonk::vulkan {
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

SamplerManager::Handle SamplerManager::get(vk::SamplerCreateInfo info) {
  auto sampler = find(info);
  if (sampler.valid())
    return sampler;

  if (mNextSlot >= MaxSamplers)
    throw std::runtime_error("Too many samplers");

  auto index = mNextSlot;
  mNextSlot++;

  auto& handle = VulkanHandle::get();
  mEntries[index].info = info;
  check(handle.mDevice.createSampler(&info, nullptr, &mEntries[index].sampler));

  DescriptorAllocator::writeSampler(mDescriptorSet, mEntries[index].sampler,
                                    index);

  return Handle(index, 0);
}

SamplerManager::Handle SamplerManager::find(vk::SamplerCreateInfo info) {
  for (int i = 0; i < mNextSlot; i++) {
    if (mEntries[i].info == info) {
      return Handle(i, 0);
    }
  }
  return Handle();
}

} // namespace selwonk::vulkan
