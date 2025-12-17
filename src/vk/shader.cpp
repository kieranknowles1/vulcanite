#include "shader.hpp"

#include "utility.hpp"
#include "vulkanengine.hpp"
#include <fmt/base.h>
#include <fstream>
#include <vulkan/vulkan_core.h>

namespace selwonk::vulkan {

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

ShaderStage::ShaderStage(Vfs::SubdirPath path, VkShaderStageFlagBits stage,
                         std::string_view entryPoint)
    : mStage(stage), mEntryPoint(entryPoint) {
  auto &vfs = VulkanEngine::get().getVfs();
  auto device = VulkanEngine::get().getVulkan().mDevice;

  auto file = vfs.open(Vfs::Shaders / path);
  file.seekg(0, std::ios::end);
  size_t size = file.tellg();
  file.seekg(0, std::ios::beg);

  // Spirv expects a u32 buffer
  std::vector<uint32_t> buffer(size / sizeof(uint32_t));
  file.read(reinterpret_cast<char *>(buffer.data()), size);

  VkShaderModuleCreateInfo info = {
      .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
      .pNext = nullptr,
      .codeSize = size,
      .pCode = buffer.data()};

  check(vkCreateShaderModule(device, &info, nullptr, &mModule));
}

ShaderStage::~ShaderStage() {
  auto device = VulkanEngine::get().getVulkan().mDevice;
  // Shader modules are not needed after the pipeline is created, so it's
  // safe to destroy them after a stack variable goes out of scope
  vkDestroyShaderModule(device, mModule, nullptr);
}

void Shader::link(VkDescriptorSetLayout layout, const ShaderStage &stage) {
  auto device = VulkanEngine::get().getVulkan().mDevice;

  VkPipelineLayoutCreateInfo layoutCreateInfo = {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
      .setLayoutCount = 1,
      .pSetLayouts = &layout,
  };
  check(vkCreatePipelineLayout(device, &layoutCreateInfo, nullptr, &mLayout));

  VkPipelineShaderStageCreateInfo stageInfo = {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
      .stage = stage.mStage,
      .module = stage.mModule,
      .pName = stage.mEntryPoint.data(),
  };

  VkComputePipelineCreateInfo pipelineInfo = {
      .sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO,
      .stage = stageInfo,
      .layout = mLayout,
  };

  check(vkCreateComputePipelines(device, nullptr, 1, &pipelineInfo, nullptr,
                                 &mPipeline));
}

void Shader::free() {
  auto device = VulkanEngine::get().getVulkan().mDevice;
  vkDestroyPipeline(device, mPipeline, nullptr);
  vkDestroyPipelineLayout(device, mLayout, nullptr);
}

} // namespace selwonk::vulkan
