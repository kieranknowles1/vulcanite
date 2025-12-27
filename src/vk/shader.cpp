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
                               std::span<PoolSizeRatio> ratios,
                               bool allowArbitraryFree) {
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

  if (allowArbitraryFree) {
    poolInfo.flags |= vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet;
  }

  check(device.createDescriptorPool(&poolInfo, nullptr, &mPool));
}

vk::DescriptorSet
DescriptorAllocator::allocateImpl(vk::DescriptorSetLayout layout) {
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

void DescriptorAllocator::freeImpl(vk::DescriptorSet set) {
  auto device = VulkanEngine::get().getVulkan().mDevice;
  check(device.freeDescriptorSets(mPool, 1, &set));
}

void DescriptorAllocator::reset() {
  auto device = VulkanEngine::get().getVulkan().mDevice;
  check(device.resetDescriptorPool(mPool, {}));
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

vk::PipelineShaderStageCreateInfo ShaderStage::createStageInfo() const {
  return vk::PipelineShaderStageCreateInfo{
      .stage = mStage,
      .module = mModule,
      .pName = mEntryPoint.data(),
  };
}

void ComputePipeline::link(vk::DescriptorSetLayout layout,
                           const ShaderStage &stage,
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

  assert(stage.mStage == vk::ShaderStageFlagBits::eCompute);
  vk::ComputePipelineCreateInfo pipelineInfo = {
      .stage = stage.createStageInfo(),
      .layout = mLayout,
  };

  check(device.createComputePipelines(nullptr, 1, &pipelineInfo, nullptr,
                                      &mPipeline));
}

void ComputePipeline::free() {
  auto device = VulkanEngine::get().getVulkan().mDevice;
  vkDestroyPipeline(device, mPipeline, nullptr);
  vkDestroyPipelineLayout(device, mLayout, nullptr);
}

Pipeline Pipeline::Builder::build(vk::Device device) {
  // TODO: Support multiple viewports/scissors
  vk::PipelineViewportStateCreateInfo viewportState = {
      .viewportCount = 1,
      .scissorCount = 1,
      // These don't need to be filled as we use dynamic viewport state
      // Minimal cost on desktop as changing these just pokes a register
      // .pScissors = ,
      // .pViewports = ,
  };

  // TODO: Support transparency
  vk::PipelineColorBlendStateCreateInfo colorBlending = {
      .logicOpEnable = false,
      .logicOp = vk::LogicOp::eCopy,
      .attachmentCount = 1,
      .pAttachments = &mColorBlendAttachment,
  };

  vk::VertexInputBindingDescription vertexInputBinding = {
      .binding = 0,
      .stride = sizeof(interop::Vertex),
      .inputRate = vk::VertexInputRate::eVertex,
  };

  vk::PipelineVertexInputStateCreateInfo vertexInputInfo = {
      .vertexBindingDescriptionCount = 1,
      .pVertexBindingDescriptions = &vertexInputBinding,
      .vertexAttributeDescriptionCount =
          static_cast<uint32_t>(mVertexInputAttributes.size()),
      .pVertexAttributeDescriptions = mVertexInputAttributes.data(),
  };

  // Define parameters that won't be hardcoded. Not too much overhead on
  // desktop and supported by almost all GPUs
  std::array<vk::DynamicState, 2> dynamicStates = {
      vk::DynamicState::eViewport,
      vk::DynamicState::eScissor,
  };
  vk::PipelineDynamicStateCreateInfo dynamicStateInfo = {
      .dynamicStateCount = static_cast<uint32_t>(dynamicStates.size()),
      .pDynamicStates = dynamicStates.data(),
  };

  // TODO: Support descriptor sets, push constants, etc
  vk::PipelineLayoutCreateInfo layoutCreateInfo = {
      .pushConstantRangeCount =
          static_cast<uint32_t>(mPushConstantRanges.size()),
      .pPushConstantRanges = mPushConstantRanges.data()};
  vk::PipelineLayout layout = {};
  check(device.createPipelineLayout(&layoutCreateInfo, nullptr, &layout));

  vk::GraphicsPipelineCreateInfo createInfo = {
      .pNext = mRenderInfo,
      .stageCount = static_cast<uint32_t>(mShaderStages.size()),
      .pStages = mShaderStages.data(),
      .pVertexInputState = &vertexInputInfo,
      .pInputAssemblyState = &mInputAssembly,
      .pViewportState = &viewportState,
      .pRasterizationState = &mRasterizer,
      .pMultisampleState = &mMultisampling,
      .pDepthStencilState = &mDepthStencil,
      .pColorBlendState = &colorBlending,
      .pDynamicState = &dynamicStateInfo,
      .layout = layout,
  };

  Pipeline pipeline;
  check(device.createGraphicsPipelines(
      /*pipelineCache=*/nullptr, /*createInfoCount*/ 1, &createInfo, nullptr,
      /*pPipelines=*/&pipeline.mPipeline));
  pipeline.mLayout = layout;
  return pipeline;
}

Pipeline::Builder &Pipeline::Builder::setShaders(const ShaderStage &vertex,
                                                 const ShaderStage &fragment) {
  assert(vertex.mStage == vk::ShaderStageFlagBits::eVertex);
  assert(fragment.mStage == vk::ShaderStageFlagBits::eFragment);
  mShaderStages[VertexIndex] = vertex.createStageInfo();
  mShaderStages[FragmentIndex] = fragment.createStageInfo();
  return *this;
}

} // namespace selwonk::vulkan
