#pragma once

#include <vncore/bumpallocator.hpp>

#include "vulkanhandle.hpp"
#include "buffer.hpp"

#include "../../assets/shaders/triangle.h"
#include "../../assets/shaders/gradient.h"
#include "shader.hpp"
#include "vulkannativehandleprovider.hpp"

namespace selwonk::vulkan {

class VulkanRenderPipeline : public core::Singleton<VulkanRenderPipeline>
{
public:
  static constexpr unsigned int FramesInFlight = 2;

  VulkanRenderPipeline(VulkanHandle& handle, core::ThreadPool& threadPool, core::Vfs& vfs);
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

  const static constexpr size_t DescriptorSetCount = 7;
  std::array<vk::DescriptorSet, DescriptorSetCount>
    getStaticDescriptors(const VulkanRenderPipeline::FrameData& frameData) {
    return {
        frameData.mSceneUniformDescriptor,
        mNativeHandles.getNativeSamplers().getDescriptorSet(),
        mNativeHandles.getNativeTextures().getDescriptorSet(),
        mNativeHandles.getNativeVertexes().getSet(),
        mNativeHandles.getNativeIndexes().getSet(),
        frameData.mInstanceDataDescriptor,
        mNativeHandles.getNativeMaterials().getSet(),
    };
  }

  std::array<vk::DescriptorSetLayout, DescriptorSetCount>
    getDescriptorLayouts() {
    return {
        mSceneUniformDescriptorLayout,
        mNativeHandles.getNativeSamplers().getDescriptorLayout(),
        mNativeHandles.getNativeTextures().getDescriptorLayout(),
        mNativeHandles.getNativeVertexes().getLayout(),
        mNativeHandles.getNativeIndexes().getLayout(),
        mInstanceDataLayout,
        mNativeHandles.getNativeMaterials().getLayout(),
    };
  }

  VulkanNativeHandleProvider& getNativeHandles() { return mNativeHandles; }

  // TODO: Temp public
//private:
  VulkanHandle& mHandle;
  VulkanNativeHandleProvider mNativeHandles;

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
