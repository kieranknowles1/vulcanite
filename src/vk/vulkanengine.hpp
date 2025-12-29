#pragma once

#include <array>

#include <SDL3/SDL_video.h>
#include <vulkan/vulkan.hpp>

#include "../vfs.hpp"
#include "imguiwrapper.hpp"
#include "mesh.hpp"
#include "shader.hpp"
#include "vulkan/vulkan.hpp"
#include "vulkanhandle.hpp"

#include "../ecs/registry.hpp"

#include "../../assets/shaders/interop.h"

namespace selwonk::vulkan {
class VulkanEngine {
public:
  struct FrameData {
    vk::CommandPool mCommandPool;     // Allocator for command buffers
    vk::CommandBuffer mCommandBuffer; // Pool of commands yet to be submitted

    vk::Semaphore
        mSwapchainSemaphore; // Tell the GPU when the GPU is done rendering
    vk::Fence mRenderFence;  // Tell the CPU when the GPU is done rendering

    DescriptorSet<StructBuffer<interop::SceneData>> mSceneUniformDescriptor;
    StructBuffer<interop::SceneData> mSceneUniforms;

    void init(VulkanHandle &handle, VulkanEngine &engine);
    void destroy(VulkanHandle &handle, VulkanEngine &engine);
  };

  static constexpr unsigned int BufferCount = 2;

  static VulkanEngine &get();

  VulkanEngine(core::Window &window);
  ~VulkanEngine();

  void init(core::Settings settings);
  void run();
  void shutdown();

  FrameData &getCurrentFrame() {
    return mFrameData[mFrameNumber % BufferCount];
  }

  VulkanHandle &getVulkan() { return mHandle; }
  Vfs &getVfs() { return *mVfs; }

private:
  void initDrawImage(glm::uvec2 size);
  void initCommands();
  void initDescriptors();
  void initTextures();

  void draw();
  void drawBackground(vk::CommandBuffer cmd);
  void drawScene(vk::CommandBuffer cmd);

  core::Settings mSettings;

  core::Window &mWindow;
  VulkanHandle mHandle;

  Image mDrawImage;
  Image mDepthImage;

  Image mMissingTexture;

  // TODO: Temp
  Image mWhite;
  Image mGrey;
  Image mBlack;

  vk::Sampler mDefaultNearestSampler;
  vk::Sampler mDefaultLinearSampler;

  // Default descriptor pool, allocations valid for the frame they are made
  DescriptorAllocator mGlobalDescriptorAllocator;
  DescriptorSet<ImageDescriptor> mDrawImageDescriptors;
  vk::DescriptorSetLayout mDrawImageDescriptorLayout;
  vk::DescriptorSetLayout mSceneUniformDescriptorLayout;

  vk::DescriptorSetLayout mTextureDescriptorLayout;
  DescriptorSet<ImageSamplerDescriptor> mTextureDescriptors;

  ImguiWrapper mImgui;

  ComputePipeline mGradientShader;
  interop::GradientPushConstants mPushConstants = {
      .leftColor = {0.0f, 0.0f, 1.0f, 1.0f},
      .rightColor = {1.0f, 0.0f, 0.0f, 1.0f},
  };

  Pipeline mTrianglePipeline;

  std::array<FrameData, BufferCount> mFrameData;

  std::unique_ptr<Vfs> mVfs;

  unsigned int mFrameNumber = 0;

  ecs::Registry mEcs;
};
} // namespace selwonk::vulkan
