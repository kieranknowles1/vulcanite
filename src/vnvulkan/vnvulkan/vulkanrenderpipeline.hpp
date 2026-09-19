#pragma once

#include <vncore/bumpallocator.hpp>

#include "vulkanhandle.hpp"
#include "buffer.hpp"

#include "../../assets/shaders/triangle.h"
#include "../../assets/shaders/gradient.h"
#include "shader.hpp"

namespace selwonk::vulkan {

class VulkanRenderPipeline
{
public:
  static constexpr unsigned int FramesInFlight = 2;

  VulkanRenderPipeline(VulkanHandle& handle, core::Vfs& vfs);
  ~VulkanRenderPipeline();

  struct FrameData {
    vk::CommandPool mCommandPool;     // Allocator for command buffers
    vk::CommandBuffer mCommandBuffer; // Pool of commands yet to be submitted

    vk::Semaphore
      mSwapchainSemaphore; // Tell the GPU when the GPU is done rendering
    vk::Fence mRenderFence;  // Tell the CPU when the GPU is done rendering

    vk::DescriptorSet mSceneUniformDescriptor;
    StructBuffer<interop::SceneData> mSceneUniforms;

    Buffer mFrameDataBuffer;
    core::BumpAllocator mFrameData;
    vk::DescriptorSet mInstanceDataDescriptor;

    void init(VulkanHandle& handle, VulkanRenderPipeline& pipeline);
    void destroy(VulkanHandle& handle, VulkanRenderPipeline& pipeline);
  };

  // TODO: Temp public
//private:
  VulkanHandle& mHandle;

  DescriptorAllocator mGlobalDescriptorAllocator;
  vk::DescriptorSetLayout mSceneUniformDescriptorLayout;
  vk::DescriptorSetLayout mInstanceDataLayout;
  vk::DescriptorSetLayout mDrawImageDescriptorLayout;
  // Default descriptor pool, allocations valid for the frame they are made
  vk::DescriptorSet mDrawImageDescriptors;

  ComputePipeline mGradientShader;
  interop::GradientPushConstants mPushConstants = {
      .leftColor = {0.0f, 0.0f, 1.0f, 1.0f},
      .rightColor = {1.0f, 0.0f, 0.0f, 1.0f},
  };

  std::array<FrameData, FramesInFlight> mFrameData;
};

}
