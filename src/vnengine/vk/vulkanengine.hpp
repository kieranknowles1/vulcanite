#pragma once

#include <array>

#include <SDL3/SDL_video.h>
#include <memory>
#include <vulkan/vulkan.hpp>

#include "buffer.hpp"
#include "bufferarray.hpp"
#include "buffermap.hpp"
#include "camerasystem.hpp"
#include "debug.hpp"
#include "imguiwrapper.hpp"
#include "material.hpp"
#include "mesh.hpp"
#include "meshloader.hpp"
#include "samplercache.hpp"
#include "shader.hpp"
#include "texturemanager.hpp"
#include "vncore/handelist.hpp"
#include "vulkan/vulkan.hpp"
#include "vulkanhandle.hpp"
#include <vncore/vfs.hpp>

#include "../ecs/registry.hpp"
#include <vncore/profiler.hpp>
#include <vncore/singleton.hpp>

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

    vk::DescriptorSet mSceneUniformDescriptor;
    // TODO: Put in same buffer as bump allocator
    // probably want to denote a "static" section that's never freed
    StructBuffer<interop::SceneData> mSceneUniforms;

    Buffer mFrameDataBuffer;
    core::BumpAllocator mFrameData;
    vk::DescriptorSet mInstanceDataDescriptor;

    void init(VulkanHandle& handle, VulkanEngine& engine);
    void destroy(VulkanHandle& handle, VulkanEngine& engine);
  };

  static constexpr unsigned int BufferCount = 2;

  VulkanEngine(core::Settings& settings, core::Window& window,
               VulkanHandle& handle);
  ~VulkanEngine();

  void run();

  VulkanHandle& getVulkan() { return mHandle; }
  core::Vfs& getVfs() { return *mVfs; }

  TextureManager::Handle getErrorTexture() {
    return mTextureManager.getMissing();
  }
  TextureManager::Handle getWhiteTexture() {
    return mTextureManager.getWhite();
  }
  SamplerCache& getSamplerCache() { return mSamplerCache; }
  TextureManager& getTextureManager() { return mTextureManager; }

  FrameData& prepareRendering();

  const static constexpr size_t DescriptorSetCount = 7;
  std::array<vk::DescriptorSet, DescriptorSetCount>
  getStaticDescriptors(const FrameData& frameData) {
    return {
        frameData.mSceneUniformDescriptor,
        mSamplerCache.getDescriptorSet(),
        mTextureManager.getDescriptorSet(),
        mVertexBuffers.getSet(),
        mIndexBuffers.getSet(),
        frameData.mInstanceDataDescriptor,
        mMaterials.getSet(),
    };
  }

  std::array<vk::DescriptorSetLayout, DescriptorSetCount>
  getDescriptorLayouts() {
    return {
        mSceneUniformDescriptorLayout,
        mSamplerCache.getDescriptorLayout(),
        mTextureManager.getDescriptorLayout(),
        mVertexBuffers.getLayout(),
        mIndexBuffers.getLayout(),
        mInstanceDataLayout,
        mMaterials.getLayout(),
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
  core::Settings& mSettings;
  core::Window& mWindow;
  VulkanHandle& mHandle;
  std::unique_ptr<core::Vfs> mVfs;
  // TODO: These are not caches, correct the names
  core::Profiler mProfiler;
  std::unique_ptr<Debug> mDebug;

  // Resources
  SamplerCache mSamplerCache;
  TextureManager mTextureManager;
  core::HandleList<Mesh> mMeshes;

  // World
  ecs::Registry mEcs;

  // Default descriptor pool, allocations valid for the frame they are made
  DescriptorAllocator mGlobalDescriptorAllocator;
  vk::DescriptorSet mDrawImageDescriptors;
  // TODO: Temp public
public:
  BufferMap mVertexBuffers;
  BufferMap mIndexBuffers;
  BufferArray<interop::MaterialData> mMaterials;

  vk::DescriptorSetLayout mDrawImageDescriptorLayout;
  vk::DescriptorSetLayout mSceneUniformDescriptorLayout;
  vk::DescriptorSetLayout mInstanceDataLayout;

  ImguiWrapper mImgui;

  ComputePipeline mGradientShader;
  interop::GradientPushConstants mPushConstants = {
      .leftColor = {0.0f, 0.0f, 1.0f, 1.0f},
      .rightColor = {1.0f, 0.0f, 0.0f, 1.0f},
  };

  bool mPipelinesDirty = true;
  Pipeline mOpaquePipeline;
  Pipeline mTranslucentPipeline;

  Material mDefaultMaterial;

  std::array<FrameData, BufferCount> mFrameData;

  unsigned int mFrameNumber = 0;

  CameraSystem* mCamera;
};
} // namespace selwonk::vulkan
