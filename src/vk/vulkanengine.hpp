#pragma once

#include <array>

#include <SDL3/SDL_video.h>
#include <memory>
#include <vulkan/vulkan.hpp>

#include "../vfs.hpp"
#include "buffermap.hpp"
#include "camerasystem.hpp"
#include "debug.hpp"
#include "imguiwrapper.hpp"
#include "material.hpp"
#include "meshloader.hpp"
#include "samplercache.hpp"
#include "shader.hpp"
#include "texturemanager.hpp"
#include "vulkan/vulkan.hpp"
#include "vulkanhandle.hpp"

#include "../core/cli.hpp"
#include "../core/profiler.hpp"
#include "../core/singleton.hpp"
#include "../ecs/registry.hpp"

#include "../../assets/shaders/gradient.h"
#include "../../assets/shaders/triangle.h"

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

  VulkanHandle& getVulkan() { return mHandle; }
  Vfs& getVfs() { return *mVfs; }

  TextureManager::Handle getErrorTexture() {
    return mTextureManager.getMissing();
  }
  TextureManager::Handle getWhiteTexture() {
    return mTextureManager.getWhite();
  }
  SamplerCache& getSamplerCache() { return mSamplerCache; }
  TextureManager& getTextureManager() { return mTextureManager; }

  FrameData& prepareRendering();

  std::array<vk::DescriptorSet, 5>
  getStaticDescriptors(const FrameData& frameData) {
    return {
        frameData.mSceneUniformDescriptor.getSet(),
        mSamplerCache.getDescriptorSet(),
        mTextureManager.getDescriptorSet(),
        mVertexBuffers.getSet(),
        mIndexBuffers.getSet(),
    };
  }

  std::array<vk::DescriptorSetLayout, 5> getDescriptorLayouts() {
    return {
        mSceneUniformDescriptorLayout,
        mSamplerCache.getDescriptorLayout(),
        mTextureManager.getDescriptorLayout(),
        mVertexBuffers.getLayout(),
        mIndexBuffers.getLayout(),
    };
  }

  BufferMap& getIndexBuffers() { return mIndexBuffers; }
  BufferMap& getVertexBuffers() { return mVertexBuffers; }

  // private:
  FrameData& getCurrentFrame() {
    return mFrameData[mFrameNumber % BufferCount];
  }

  struct CameraImages {
    std::shared_ptr<Image> draw;
    std::shared_ptr<Image> depth;
  };
  const static constexpr vk::Format DrawFormat =
      vk::Format::eR16G16B16A16Sfloat;
  const static constexpr vk::Format DepthFormat = vk::Format::eD32Sfloat;

  CameraImages initDrawImage(glm::uvec2 size);
  void initCommands();
  void initDescriptors();

  void initPipelines();
  void initEcs();

  void writeBackgroundDescriptors();

  void present();

  // Sub systems
  const core::Cli& mCli;
  core::Settings& mSettings;
  core::Window& mWindow;
  VulkanHandle& mHandle;
  ecs::Registry mEcs;
  std::unique_ptr<Vfs> mVfs;
  // TODO: These are not caches, correct the names
  SamplerCache mSamplerCache;
  TextureManager mTextureManager;
  core::Profiler mProfiler;
  std::unique_ptr<Debug> mDebug;

  // Default descriptor pool, allocations valid for the frame they are made
  DescriptorAllocator mGlobalDescriptorAllocator;
  DescriptorSet<ImageDescriptor> mDrawImageDescriptors;
  // TODO: Temp public
public:
  BufferMap mVertexBuffers;
  BufferMap mIndexBuffers;

  vk::DescriptorSetLayout mDrawImageDescriptorLayout;
  vk::DescriptorSetLayout mSceneUniformDescriptorLayout;

  ImguiWrapper mImgui;

  ComputePipeline mGradientShader;
  interop::GradientPushConstants mPushConstants = {
      .leftColor = {0.0f, 0.0f, 1.0f, 1.0f},
      .rightColor = {1.0f, 0.0f, 0.0f, 1.0f},
  };

  bool mPipelinesDirty = true;
  Pipeline mOpaquePipeline;
  Pipeline mTranslucentPipeline;

  std::shared_ptr<Material> mDefaultMaterial;
  StructBuffer<interop::MaterialData> mDefaultMaterialData;

  std::array<FrameData, BufferCount> mFrameData;

  std::unique_ptr<GltfMesh> mMesh;

  unsigned int mFrameNumber = 0;

  CameraSystem* mCamera;
};
} // namespace selwonk::vulkan
