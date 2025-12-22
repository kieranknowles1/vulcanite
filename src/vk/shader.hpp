#pragma once

#include <span>
#include <string_view>
#include <vector>

#include <vulkan/vulkan.hpp>

#include "../vfs.hpp"

namespace selwonk::vulkan {

// Describe the layout of a descriptor set. That is, the types and counts of a
// shader's bindings (inputs and outputs)
class DescriptorLayoutBuilder {
public:
  void addBinding(uint32_t binding, vk::DescriptorType type);
  vk::DescriptorSetLayout build(vk::Device device, vk::ShaderStageFlags stages,
                                void *pNext = nullptr,
                                vk::DescriptorSetLayoutCreateFlags flags = {});

private:
  std::vector<vk::DescriptorSetLayoutBinding> bindings;
};

class DescriptorAllocator {
public:
  struct PoolSizeRatio {
    vk::DescriptorType type;
    // How many descriptors of this type to allocate per set
    float ratio;
  };

  void init(uint32_t maxSets, std::span<PoolSizeRatio> ratios);
  void destroy();

  vk::DescriptorSet allocate(vk::DescriptorSetLayout layout);

private:
  vk::DescriptorPool mPool;
};

class ShaderStage {
public:
  ShaderStage(Vfs::SubdirPath path, vk::ShaderStageFlagBits stage,
              std::string_view entryPoint);
  ~ShaderStage();

  vk::ShaderModule mModule;
  vk::ShaderStageFlagBits mStage;
  std::string_view mEntryPoint;
};

class ComputePipeline {
public:
  void link(vk::DescriptorSetLayout layout, const ShaderStage &stage,
            uint32_t pushConstantsSize);
  void free();

  // TODO
  // private:
  vk::Pipeline mPipeline;
  vk::PipelineLayout mLayout;
};

} // namespace selwonk::vulkan
