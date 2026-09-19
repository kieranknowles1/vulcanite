#pragma once

#include <array>

#include <memory>
#include <vulkan/vulkan.hpp>

#include <vnvulkan/buffer.hpp>
#include <vnvulkan/bufferarray.hpp>
#include <vnvulkan/buffermap.hpp>
#include "../ecs/camerasystem.hpp"
#include <vnassets/debug.hpp>
#include <vnvulkan/imguiwrapper.hpp>
#include <vnvulkan/shader.hpp>
#include "vncore/threadpool.hpp"
#include <vnvulkan/vulkanhandle.hpp>
#include <vncore/vfs.hpp>

#include "../ui/cvarui.hpp"
#include "../ui/profilerui.hpp"
#include <vnecs/registry.hpp>
#include <vncore/profiler.hpp>
#include <vnvulkan/vulkanrenderpipeline.hpp>
#include <vncore/singleton.hpp>


namespace selwonk::vulkan {
class VulkanEngine : public core::Singleton<VulkanEngine> {
public:
  VulkanEngine(sdl::Window& window, VulkanHandle& handle);
  ~VulkanEngine();

  void run();

  VulkanHandle& getVulkan() { return mPipeline->mHandle; }
  core::Vfs& getVfs() const { return *mVfs; }

  VulkanRenderPipeline::FrameData& prepareRendering();

  const static constexpr size_t DescriptorSetCount = 7;
  [[deprecated]]
  std::array<vk::DescriptorSet, DescriptorSetCount>
  getStaticDescriptors(const VulkanRenderPipeline::FrameData& frameData) {
    return mPipeline->getStaticDescriptors(frameData);
  }

  [[deprecated]]
  std::array<vk::DescriptorSetLayout, DescriptorSetCount>
  getDescriptorLayouts() {
    return mPipeline->getDescriptorLayouts();
  }

  core::ThreadPool& getThreadPool() { return mThreadPool; }

  // TODO: Make this private
  // private:
  VulkanRenderPipeline::FrameData& getCurrentFrame() {
    return mPipeline->mFrameData[mFrameNumber % VulkanRenderPipeline::FramesInFlight];
  }

  struct CameraImages {
    TextureManager::Handle draw;
    TextureManager::Handle depth;
  };
  const static constexpr vk::Format DrawFormat =
      vk::Format::eR16G16B16A16Sfloat;
  const static constexpr vk::Format DepthFormat = vk::Format::eD32Sfloat;

  CameraImages initDrawImage(glm::uvec2 size);

  void initPipelines();
  void initEcs();

  void writeBackgroundDescriptors();

  void present();

  [[deprecated(
      "Use only as a last resort, promote missing features to interface")]]
  VulkanNativeHandleProvider& getNativeHandles() {
    return mPipeline->mNativeHandles;
  }

  std::unique_ptr<VulkanRenderPipeline> mPipeline;

  // Sub systems
  core::ThreadPool mThreadPool;
  sdl::Window& mWindow;
  std::unique_ptr<core::Vfs> mVfs;
  core::Profiler mProfiler;
  ui::ProfilerUi mProfilerUi;

  std::unique_ptr<ui::CvarUi> mCvarUi;

  // World
  ecs::Registry mEcs;
  assets::Debug mDebug;

  // TODO: Temp public
public:

  ImguiWrapper mImgui;

  bool mPipelinesDirty = true;
  Pipeline mOpaquePipeline;
  Pipeline mTranslucentPipeline;

  unsigned int mFrameNumber = 0;

  ecs::CameraSystem* mCamera;

  bool mConsoleVisible = true;
};
} // namespace selwonk::vulkan
