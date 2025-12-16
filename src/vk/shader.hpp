#pragma once

#include <span>
#include <vector>

#include <vulkan/vulkan.h>

namespace selwonk::vk {

// Describe the layout of a descriptor set. That is, the types and counts of a
// shader's bindings (inputs and outputs)
class DescriptorLayoutBuilder {
public:
  void addBinding(uint32_t binding, VkDescriptorType type);
  VkDescriptorSetLayout build(VkDevice device, VkShaderStageFlags stages,
                              void *pNext = nullptr,
                              VkDescriptorSetLayoutCreateFlags flags = 0);

private:
  std::vector<VkDescriptorSetLayoutBinding> bindings;
};

class DescriptorAllocator {
public:
  struct PoolSizeRatio {
    VkDescriptorType type;
    // How many descriptors of this type to allocate per set
    float ratio;
  };

  void init(uint32_t maxSets, std::span<PoolSizeRatio> ratios);
  void destroy();

  VkDescriptorSet allocate(VkDescriptorSetLayout layout);

private:
  VkDescriptorPool mPool;
};

} // namespace selwonk::vk
