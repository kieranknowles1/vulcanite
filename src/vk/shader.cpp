#include "shader.hpp"

#include "utility.hpp"
#include "vulkan/vulkan.hpp"
#include "vulkanengine.hpp"
#include <cstdint>
#include <fmt/base.h>
#include <fstream>

namespace selwonk::vulkan {

void DescriptorLayoutBuilder::addBinding(uint32_t binding,
                                         vk::DescriptorType type) {
  vk::DescriptorSetLayoutBinding bind = {
      .binding = binding,
      .descriptorType = type,
      .descriptorCount = 1,
  };

  bindings.push_back(bind);
}

vk::DescriptorSetLayout
DescriptorLayoutBuilder::build(vk::Device device, vk::ShaderStageFlags stages,
                               void *pNext,
                               vk::DescriptorSetLayoutCreateFlags flags) {
  for (auto &binding : bindings) {
    binding.stageFlags |= stages;
  }

  vk::DescriptorSetLayoutCreateInfo info = {
      .pNext = pNext,
      .flags = flags,
      .bindingCount = static_cast<uint32_t>(bindings.size()),
      .pBindings = bindings.data(),
  };

  vk::DescriptorSetLayout set;
  check(device.createDescriptorSetLayout(&info, nullptr, &set));
  return set;
}

void DescriptorAllocator::init(uint32_t maxSets,
                               std::span<PoolSizeRatio> ratios) {
  auto device = VulkanEngine::get().getVulkan().mDevice;

  std::vector<vk::DescriptorPoolSize> sizes;
  sizes.reserve(ratios.size());
  for (auto &ratio : ratios) {
    uint32_t count = uint32_t(ratio.ratio * maxSets);
    assert(count > 0 && "Descriptor count must be greater than zero");
    sizes.push_back(vk::DescriptorPoolSize{
        .type = ratio.type,
        .descriptorCount = count,
    });
  }

  vk::DescriptorPoolCreateInfo poolInfo = {
      .maxSets = maxSets,
      .poolSizeCount = static_cast<uint32_t>(sizes.size()),
      .pPoolSizes = sizes.data(),
  };

  check(device.createDescriptorPool(&poolInfo, nullptr, &mPool));
}

vk::DescriptorSet
DescriptorAllocator::allocate(vk::DescriptorSetLayout layout) {
  auto device = VulkanEngine::get().getVulkan().mDevice;
  vk::DescriptorSetAllocateInfo info = {
      .descriptorPool = mPool,
      .descriptorSetCount = 1,
      .pSetLayouts = &layout,
  };

  vk::DescriptorSet set;
  check(device.allocateDescriptorSets(&info, &set));
  return set;
}

void DescriptorAllocator::destroy() {
  auto device = VulkanEngine::get().getVulkan().mDevice;
  device.destroyDescriptorPool(mPool, nullptr);
}

ShaderStage::ShaderStage(Vfs::SubdirPath path, vk::ShaderStageFlagBits stage,
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

  vk::ShaderModuleCreateInfo info = {
      .pNext = nullptr,
      .codeSize = size,
      .pCode = buffer.data(),
  };

  check(device.createShaderModule(&info, nullptr, &mModule));
}

ShaderStage::~ShaderStage() {
  auto device = VulkanEngine::get().getVulkan().mDevice;
  // Shader modules are not needed after the pipeline is created, so it's
  // safe to destroy them after a stack variable goes out of scope
  vkDestroyShaderModule(device, mModule, nullptr);
}

void Shader::link(vk::DescriptorSetLayout layout, const ShaderStage &stage,
                  uint32_t pushConstantsSize) {
  assert(pushConstantsSize <= 128 &&
         "Push constants larger than 128 bytes may not be supported");
  auto device = VulkanEngine::get().getVulkan().mDevice;

  vk::PushConstantRange pushConstant{
      .stageFlags = stage.mStage,
      .offset = 0,
      .size = pushConstantsSize,
  };

  vk::PipelineLayoutCreateInfo layoutCreateInfo = {
      .setLayoutCount = 1,
      .pSetLayouts = &layout,
      .pushConstantRangeCount = 1,
      .pPushConstantRanges = &pushConstant,
  };
  check(device.createPipelineLayout(&layoutCreateInfo, nullptr, &mLayout));

  vk::PipelineShaderStageCreateInfo stageInfo = {
      .stage = stage.mStage,
      .module = stage.mModule,
      .pName = stage.mEntryPoint.data(),
  };

  vk::ComputePipelineCreateInfo pipelineInfo = {
      .stage = stageInfo,
      .layout = mLayout,
  };

  check(device.createComputePipelines(nullptr, 1, &pipelineInfo, nullptr,
                                      &mPipeline));
}

void Shader::free() {
  auto device = VulkanEngine::get().getVulkan().mDevice;
  vkDestroyPipeline(device, mPipeline, nullptr);
  vkDestroyPipelineLayout(device, mLayout, nullptr);
}

} // namespace selwonk::vulkan
