#pragma once

#include <array>

#include <SDL3/SDL_video.h>
#include <memory>
#include <vulkan/vulkan.hpp>

#include "../vfs.hpp"
#include "imguiwrapper.hpp"
#include "material.hpp"
#include "meshloader.hpp"
#include "samplercache.hpp"
#include "shader.hpp"
#include "vulkan/vulkan.hpp"
#include "vulkanhandle.hpp"

#include "../core/cli.hpp"
#include "../core/profiler.hpp"
#include "../core/singleton.hpp"
#include "../ecs/registry.hpp"

#include "../../assets/shaders/interop.h"

namespace selwonk::vulkan {
class VulkanEngine : public core::Singleton<VulkanEngine> {
public:
  struct FrameData {
    vk::CommandPool mCommandPool;     // Allocator for command buffers
    vk::CommandBuffer mCommandBuffer; // Pool of commands yet to be submitted

    vk::Semaphore
        mSwapchainSemaphore; // Tell the GPU when the GPU is done rendering
    vk::Fence mRenderFence;  // Tell the CPU when the GPU is done rendering

    DescriptorSet<StructBuffer<interop::SceneData>> mSceneUniformDescriptor;
    StructBuffer<interop::SceneData> mSceneUniforms;

    void init(VulkanHandle& handle, VulkanEngine& engine);
    void destroy(VulkanHandle& handle, VulkanEngine& engine);
  };

  static constexpr unsigned int BufferCount = 2;

  VulkanEngine(const core::Cli& cli, core::Settings& settings,
               core::Window& window, VulkanHandle& handle);
  ~VulkanEngine();

  void run();

  FrameData& getCurrentFrame() {
    return mFrameData[mFrameNumber % BufferCount];
  }

  VulkanHandle& getVulkan() { return mHandle; }
  Vfs& getVfs() { return *mVfs; }

  std::shared_ptr<Image> getErrorTexture() { return mMissingTexture; }
  vk::DescriptorSetLayout getTextureDescriptorLayout() {
    return mTextureDescriptorLayout;
  }
  std::shared_ptr<Image> getWhiteTexture() { return mWhite; }
  SamplerCache& getSamplerCache() { return mSamplerCache; }

private:
  struct CameraImages {
    std::shared_ptr<Image> draw;
    std::shared_ptr<Image> depth;
  };
  CameraImages initDrawImage(glm::uvec2 size);
  void initCommands();
  void initDescriptors();
  void initTextures();

  void draw();
  void drawBackground(vk::CommandBuffer cmd);
  void drawScene(vk::CommandBuffer cmd);

  void present();

  // Sub systems
  const core::Cli& mCli;
  core::Settings& mSettings;
  core::Window& mWindow;
  VulkanHandle& mHandle;
  ecs::Registry mEcs;
  std::unique_ptr<Vfs> mVfs;
  SamplerCache mSamplerCache;
  core::Profiler mProfiler;

  std::shared_ptr<Image> mMissingTexture;
  // Used for textureless objects
  std::shared_ptr<Image> mWhite;

  // Default descriptor pool, allocations valid for the frame they are made
  DescriptorAllocator mGlobalDescriptorAllocator;
  DescriptorSet<ImageDescriptor> mDrawImageDescriptors;
  DescriptorSet<ImageDescriptor> mWhiteDescriptor;
  vk::DescriptorSetLayout mDrawImageDescriptorLayout;
  vk::DescriptorSetLayout mSceneUniformDescriptorLayout;

  vk::DescriptorSetLayout mTextureDescriptorLayout;

  ImguiWrapper mImgui;

  ComputePipeline mGradientShader;
  interop::GradientPushConstants mPushConstants = {
      .leftColor = {0.0f, 0.0f, 1.0f, 1.0f},
      .rightColor = {1.0f, 0.0f, 0.0f, 1.0f},
  };

  Pipeline mOpaquePipeline;
  Pipeline mTranslucentPipeline;

  Material mDefaultMaterial;
  StructBuffer<interop::MaterialData> mDefaultMaterialData;

  std::array<FrameData, BufferCount> mFrameData;

  std::unique_ptr<GltfMesh> mMesh;

  unsigned int mFrameNumber = 0;
  ecs::EntityRef mPlayerCamera;
  float mCameraSpeed = 1.0f;
  float mPitch = 0.0f;
  float mYaw = 0.0f;
};
} // namespace selwonk::vulkan
