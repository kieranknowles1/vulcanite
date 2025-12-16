#pragma once

#include <span>
#include <string_view>
#include <vector>

#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

#include "../vfs.hpp"

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

class ShaderStage {
public:
  ShaderStage(Vfs::SubdirPath path, VkShaderStageFlagBits stage,
              std::string_view name);
  ~ShaderStage();

  VkShaderModule mModule;
  VkShaderStageFlagBits mStage;
  std::string_view mEntryPoint;
};

class Shader {
public:
  void link(VkDescriptorSetLayout layout, const ShaderStage &stage);
  void free();

  // TODO
  // private:
  VkPipeline mPipeline;
  VkPipelineLayout mLayout;
};

} // namespace selwonk::vk
