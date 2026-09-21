#pragma once

#include <vncore/bumpallocator.hpp>
#include <vnecs/camera.hpp>

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

  const static constexpr vk::Format DrawFormat =
    vk::Format::eR16G16B16A16Sfloat;
  const static constexpr vk::Format DepthFormat = vk::Format::eD32Sfloat;

  VulkanRenderPipeline(
    VulkanHandle& handle,
    sdl::Window& window,
    core::ThreadPool& threadPool,
    core::Vfs& vfs);
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

  // Create a set of images for use as a camera's target
  ecs::Camera::Images createDrawImage(glm::uvec2 size);

  VulkanNativeHandleProvider& getNativeHandles() { return mNativeHandles; }
  sdl::Window& getWindow() { return mWindow; }

  FrameData& getCurrentFrame() {
    return mFrameData[mFrameNumber % VulkanRenderPipeline::FramesInFlight];
  }

  Pipeline& getOpaquePipeline() { return mOpaquePipeline; }
  Pipeline& getTranslucentPipeline() { return mTranslucentPipeline; };

  // TODO: Temp public
//private:
  void initPipelines();

  VulkanHandle& mHandle;
  sdl::Window& mWindow;
  core::Vfs& mVfs;
  VulkanNativeHandleProvider mNativeHandles;

  DescriptorAllocator mGlobalDescriptorAllocator;
  vk::DescriptorSetLayout mSceneUniformDescriptorLayout;
  vk::DescriptorSetLayout mInstanceDataLayout;
  vk::DescriptorSetLayout mDrawImageDescriptorLayout;
  // Default descriptor pool, allocations valid for the frame they are made
  vk::DescriptorSet mDrawImageDescriptors;

  bool mPipelinesDirty = true;
  Pipeline mOpaquePipeline;
  Pipeline mTranslucentPipeline;

  ComputePipeline mGradientShader;
  interop::GradientPushConstants mPushConstants = {
      .leftColor = {0.0f, 0.0f, 1.0f, 1.0f},
      .rightColor = {1.0f, 0.0f, 0.0f, 1.0f},
  };

  unsigned int mFrameNumber = 0;
  std::array<FrameData, FramesInFlight> mFrameData;
};

}
