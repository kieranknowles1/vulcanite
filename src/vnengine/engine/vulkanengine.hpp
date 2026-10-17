#pragma once

#include <memory>

#include "../ecs/camerasystem.hpp"
#include "vncore/threadpool.hpp"
#include <vnassets/debug.hpp>
#include <vncore/vfs.hpp>

#include "../ui/element.hpp"
#include <vncore/profiler.hpp>
#include <vncore/singleton.hpp>
#include <vnecs/registry.hpp>
#include <vnassets/irenderpipeline.hpp>

namespace selwonk::engine {
class VulkanEngine : public core::Singleton<VulkanEngine> {
public:
  // Common components that are created by the engine, but are also needed by platform implementations
  struct Components {
    Components();
    core::ThreadPool threadPool;
    std::unique_ptr<core::Vfs> vfs;
  };

  VulkanEngine(Components& components, assets::IRenderPipeline& pipeline, sdl::Window& window);
  ~VulkanEngine();

  void run();

  core::Vfs& getVfs() const { return *mComponents.vfs; }

  core::ThreadPool& getThreadPool() { return mComponents.threadPool; }

private:
  void initEcs();

  assets::IRenderPipeline& mPipeline;

  // Sub systems
  Components& mComponents;
  sdl::Window& mWindow;
  core::Profiler mProfiler;

  bool mConsoleVisible = true;
  std::vector<std::unique_ptr<ui::Element>> mUi;

  // World
  ecs::Registry mEcs;
  assets::Debug mDebug;

  ecs::CameraSystem* mCamera;
};
} // namespace selwonk::vulkan
