#pragma once

#include <cstdint>
#include <span>
#include <string_view>
#include <vector>

#include <vulkan/vulkan.hpp>

#include "../vfs.hpp"
#include "vulkan/vulkan.hpp"

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

// Strongly wrapped descriptor set
// T represents the data that will be written, and must provide a `write` method
// implementation with the following signature:
// void write(vk::Device device, vk::DescriptorSet set) const;
template <typename T> class DescriptorSet {
public:
  DescriptorSet(vk::DescriptorSet set) : mSet(set) {}
  DescriptorSet() = default;

  void write(vk::Device device, const T &data) { data.write(device, mSet); }

  vk::DescriptorSet &getSet() { return mSet; }

private:
  vk::DescriptorSet mSet;
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

  template <typename T>
  DescriptorSet<T> allocate(vk::DescriptorSetLayout layout) {
    return DescriptorSet<T>(allocateImpl(layout));
  }

  // Reset the pool, freeing all allocated resources
  void reset();

private:
  vk::DescriptorSet allocateImpl(vk::DescriptorSetLayout layout);
  vk::DescriptorPool mPool;
};

class ShaderStage {
public:
  ShaderStage(Vfs::SubdirPath path, vk::ShaderStageFlagBits stage,
              std::string_view entryPoint);
  ~ShaderStage();

  vk::PipelineShaderStageCreateInfo createStageInfo() const;

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

class Pipeline {
public:
  class Builder {
    const static constexpr vk::ColorComponentFlags WriteRGBA =
        vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG |
        vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA;

  public:
    Pipeline build(vk::Device device);

    Builder &setShaders(const ShaderStage &vertex, const ShaderStage &fragment);
    // Point, line, or triangle input
    Builder &setInputTopology(vk::PrimitiveTopology topology) {
      mInputAssembly.topology = topology;
      // We don't use this
      mInputAssembly.primitiveRestartEnable = false;
      return *this;
    }
    // Wireframe/solid/point output
    Builder &setPolygonMode(vk::PolygonMode mode) {
      mRasterizer.polygonMode = mode;
      mRasterizer.lineWidth = 1.0f;
      return *this;
    }
    // Front/back face culling
    Builder &setCullMode(vk::CullModeFlags mode, vk::FrontFace frontFace) {
      mRasterizer.cullMode = mode;
      mRasterizer.frontFace = frontFace;
      return *this;
    }
    Builder &disableMultisampling() {
      mMultisampling.sampleShadingEnable = false;
      mMultisampling.rasterizationSamples = vk::SampleCountFlagBits::e1;
      mMultisampling.minSampleShading = 1.0f;
      mMultisampling.pSampleMask = nullptr;
      mMultisampling.alphaToCoverageEnable = false;
      mMultisampling.alphaToOneEnable = false;
      return *this;
    }
    Builder &disableBlending() {
      mColorBlendAttachment.colorWriteMask = WriteRGBA;
      return *this;
    }
    // Additive blending, mostly used for deferred lighting
    // outColor = inColor*inAlpha + prevOutColor
    Builder &enableAdditiveBlending() {
      mColorBlendAttachment.colorWriteMask = WriteRGBA;
      mColorBlendAttachment.blendEnable = true;
      mColorBlendAttachment.srcColorBlendFactor = vk::BlendFactor::eSrcAlpha;
      mColorBlendAttachment.dstColorBlendFactor = vk::BlendFactor::eOne;
      mColorBlendAttachment.colorBlendOp = vk::BlendOp::eAdd;
      mColorBlendAttachment.srcAlphaBlendFactor = vk::BlendFactor::eOne;
      mColorBlendAttachment.dstAlphaBlendFactor = vk::BlendFactor::eZero;
      mColorBlendAttachment.alphaBlendOp = vk::BlendOp::eAdd;
      return *this;
    }
    // Alpha blend, used for transparent objects (most cases)
    // outColor = inColor*inAlpha + prevOutColor*(1-inAlpha)
    Builder &enableAlphaBlend() {
      mColorBlendAttachment.colorWriteMask = WriteRGBA;
      mColorBlendAttachment.blendEnable = true;
      mColorBlendAttachment.srcColorBlendFactor = vk::BlendFactor::eSrcAlpha;
      mColorBlendAttachment.dstColorBlendFactor =
          vk::BlendFactor::eOneMinusSrcAlpha;
      mColorBlendAttachment.colorBlendOp = vk::BlendOp::eAdd;
      mColorBlendAttachment.srcAlphaBlendFactor = vk::BlendFactor::eOne;
      mColorBlendAttachment.dstAlphaBlendFactor = vk::BlendFactor::eZero;
      mColorBlendAttachment.alphaBlendOp = vk::BlendOp::eAdd;
      return *this;
    }

    Builder &setColorAttachFormat(vk::Format format) {
      mColorFormat = format;
      return *this;
    }
    Builder &setDepthFormat(vk::Format format) {
      mRenderInfo.depthAttachmentFormat = format;
      return *this;
    }
    Builder &disableDepth() {
      mDepthStencil.depthTestEnable = false;
      mDepthStencil.depthWriteEnable = false;
      mDepthStencil.depthCompareOp = vk::CompareOp::eNever;
      mDepthStencil.depthBoundsTestEnable = false;
      mDepthStencil.front = {};
      mDepthStencil.back = {};
      mDepthStencil.minDepthBounds = 0.0f;
      mDepthStencil.maxDepthBounds = 1.0f;
      return *this;
    }
    Builder &enableDepth(bool depthWriteEnable, vk::CompareOp op) {
      mDepthStencil.depthTestEnable = true;
      mDepthStencil.depthWriteEnable = depthWriteEnable;
      mDepthStencil.depthCompareOp = op;
      mDepthStencil.depthBoundsTestEnable = false;
      mDepthStencil.front = {};
      mDepthStencil.back = {};
      mDepthStencil.minDepthBounds = 0.0f;
      mDepthStencil.maxDepthBounds = 1.0f;
      return *this;
    }

    const static constexpr vk::Format InputFloat4 =
        vk::Format::eR32G32B32A32Sfloat;
    const static constexpr vk::Format InputFloat2 = vk::Format::eR32G32Sfloat;
    Builder &addInputAttribute(vk::VertexInputAttributeDescription attribute) {
      mVertexInputAttributes.push_back(attribute);
      return *this;
    }
    Builder &setPushConstantSize(vk::ShaderStageFlagBits stage, uint32_t size) {
      mPushConstantRanges.push_back(vk::PushConstantRange{
          .stageFlags = stage,
          .offset = 0,
          .size = size,
      });
      return *this;
    }

  private:
    static constexpr size_t VertexIndex = 0;
    static constexpr size_t FragmentIndex = 1;
    std::array<vk::PipelineShaderStageCreateInfo, 2> mShaderStages;
    std::vector<vk::VertexInputAttributeDescription> mVertexInputAttributes;
    std::vector<vk::PushConstantRange> mPushConstantRanges;

    // Triangle topology config
    vk::PipelineInputAssemblyStateCreateInfo mInputAssembly = {};
    // Triangle rasterization config, fixed-function hardware between vertex
    // and fragment shader
    vk::PipelineRasterizationStateCreateInfo mRasterizer = {};
    // Control how colours are blended, similar to `glBlend*`
    vk::PipelineColorBlendAttachmentState mColorBlendAttachment = {};
    // MSAA config
    vk::PipelineMultisampleStateCreateInfo mMultisampling = {};
    // Depth and stencil testing config
    vk::PipelineDepthStencilStateCreateInfo mDepthStencil = {};
    // Attach formats we will use for dynamic rendering, passed in `pNext` of
    // GraphicsPipelineCreateInfo
    vk::PipelineRenderingCreateInfo mRenderInfo = {
        // TODO: Support multiple colour attachments for deferred rendering
        .colorAttachmentCount = 1,
        .pColorAttachmentFormats = &mColorFormat,
    };
    vk::Format mColorFormat = {};
  };

  void destroy(vk::Device device) {
    device.destroyPipeline(mPipeline, nullptr);
    device.destroyPipelineLayout(mLayout, nullptr);
  }

  vk::Pipeline getPipeline() const { return mPipeline; }
  vk::PipelineLayout getLayout() const { return mLayout; }

private:
  vk::PipelineLayout mLayout;
  vk::Pipeline mPipeline;
};

} // namespace selwonk::vulkan
