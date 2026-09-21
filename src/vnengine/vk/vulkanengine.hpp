#pragma once

#include <array>

#include <memory>
#include <vulkan/vulkan.hpp>

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

  core::ThreadPool& getThreadPool() { return mThreadPool; }

  // TODO: Make this private
  // private:

  void initEcs();

  void writeBackgroundDescriptors();

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

  ecs::CameraSystem* mCamera;

  bool mConsoleVisible = true;
};
} // namespace selwonk::vulkan
