#include "shader.hpp"

#include "utility.hpp"
#include "vulkanengine.hpp"
#include <fmt/base.h>
#include <vulkan/vulkan_core.h>

namespace selwonk::vk {

void DescriptorLayoutBuilder::addBinding(uint32_t binding,
                                         VkDescriptorType type) {
  VkDescriptorSetLayoutBinding bind = {
      .binding = binding,
      .descriptorType = type,
      .descriptorCount = 1,
  };

  bindings.push_back(bind);
}

VkDescriptorSetLayout
DescriptorLayoutBuilder::build(VkDevice device, VkShaderStageFlags stages,
                               void *pNext,
                               VkDescriptorSetLayoutCreateFlags flags) {
  for (auto &binding : bindings) {
    binding.stageFlags |= stages;
  }

  VkDescriptorSetLayoutCreateInfo info = {
      .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
      .pNext = pNext,
      .flags = flags,
      .bindingCount = static_cast<uint32_t>(bindings.size()),
      .pBindings = bindings.data(),
  };

  VkDescriptorSetLayout set;
  check(vkCreateDescriptorSetLayout(device, &info, nullptr, &set));
  return set;
}

void DescriptorAllocator::init(uint32_t maxSets,
                               std::span<PoolSizeRatio> ratios) {
  auto device = VulkanEngine::get().getVulkan().mDevice;

  std::vector<VkDescriptorPoolSize> sizes;
  sizes.reserve(ratios.size());
  for (auto &ratio : ratios) {
    uint32_t count = uint32_t(ratio.ratio * maxSets);
    assert(count > 0 && "Descriptor count must be greater than zero");
    sizes.push_back(VkDescriptorPoolSize{
        .type = ratio.type,
        .descriptorCount = count,
    });
  }

  VkDescriptorPoolCreateInfo poolInfo = {
      .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
      .maxSets = maxSets,
      .poolSizeCount = static_cast<uint32_t>(sizes.size()),
      .pPoolSizes = sizes.data(),
  };

  vkCreateDescriptorPool(device, &poolInfo, nullptr, &mPool);
}

VkDescriptorSet DescriptorAllocator::allocate(VkDescriptorSetLayout layout) {
  auto device = VulkanEngine::get().getVulkan().mDevice;
  VkDescriptorSetAllocateInfo info = {
      .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
      .descriptorPool = mPool,
      .descriptorSetCount = 1,
      .pSetLayouts = &layout,
  };

  VkDescriptorSet set;
  check(vkAllocateDescriptorSets(device, &info, &set));
  return set;
}

void DescriptorAllocator::destroy() {
  auto device = VulkanEngine::get().getVulkan().mDevice;
  vkDestroyDescriptorPool(device, mPool, nullptr);
}

} // namespace selwonk::vk
