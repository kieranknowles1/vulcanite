#pragma once

#include <memory>

#include "../ecs/camerasystem.hpp"
#include "vncore/threadpool.hpp"
#include <vnassets/debug.hpp>
#include <vncore/vfs.hpp>
#include <vnvulkan/vulkanhandle.hpp>

#include "../ui/element.hpp"
#include <vncore/profiler.hpp>
#include <vncore/singleton.hpp>
#include <vnecs/registry.hpp>
#include <vnvulkan/vulkanrenderpipeline.hpp>

namespace selwonk::engine {
class VulkanEngine : public core::Singleton<VulkanEngine> {
public:
  VulkanEngine(sdl::Window& window, vulkan::VulkanHandle& handle);
  ~VulkanEngine();

  void run();

  core::Vfs& getVfs() const { return *mVfs; }

  core::ThreadPool& getThreadPool() { return mThreadPool; }

private:
  void initEcs();

  std::unique_ptr<vulkan::VulkanRenderPipeline> mPipeline;

  // Sub systems
  core::ThreadPool mThreadPool;
  sdl::Window& mWindow;
  std::unique_ptr<core::Vfs> mVfs;
  core::Profiler mProfiler;

  bool mConsoleVisible = true;
  std::vector<std::unique_ptr<ui::Element>> mUi;

  // World
  ecs::Registry mEcs;
  assets::Debug mDebug;

  ecs::CameraSystem* mCamera;
};
} // namespace selwonk::vulkan
