#include "vulkanengine.hpp"

// #include "../ecs/camerapathsystem.hpp"
#include "vncore/vfs.hpp"
#include <vnassets/meshloader.hpp>
#include <vncore/cvar.hpp>
#include <vncore/platform.hpp>
#include <vncore/times.hpp>
#include <vnecs/util/meshinst.hpp>
#include <vnvulkan/rendersystem.hpp>
#include <vnvulkan/vulkanhandle.hpp>

#include <chrono>

#include <backends/imgui_impl_vulkan.h>
#include <glm/glm.hpp>
#include <glm/trigonometric.hpp>
#include <imgui.h>
#include <memory>
#include <spdlog/spdlog.h>

#include "../ui/cvarui.hpp"
#include "../ui/profilerui.hpp"
#include "../ui/sceneviewui.hpp"
#include "../ui/usageui.hpp"

namespace selwonk::vulkan {

core::Cvar::Int QuitAfterFrames("debug.quit_after", -1,
                                "Quit after number of frames if >= 0",
                                core::Cvar::Flags::InitOnly);
core::Cvar::Float
    FixedTimestep("physics.fixed_timestep", 0,
                  "If not zero, fixed delta time for per-frame updates");

static unsigned int getDefaultThreadCount() {
  return std::max(1u, std::thread::hardware_concurrency());
}

// TODO: Set based on CPU count
// TODO: Unsigned flag
core::Cvar::Int WorkerThreads(
    "core.worker_threads", getDefaultThreadCount, "${cpu_thread_count}",
    "Count of generic worker threads to spawn. If zero, run everything "
    "on the main thread.",
    core::util::combineFlags(core::Cvar::Flags::InitOnly,
                             core::Cvar::Flags::Unsigned));

static std::string defaultDataDir() {
  auto path = core::Platform::getExePath().parent_path() / "assets";
  return path.string();
}

core::Cvar::String DataDirectory("core.data_directory", defaultDataDir,
                                 "${exe_directory}/assets",
                                 "Path of data directory",
                                 core::Cvar::Flags::InitOnly);

VulkanEngine::VulkanEngine(sdl::Window& window, VulkanHandle& handle)
    : mThreadPool(WorkerThreads.value()), mWindow(window) {

  SPDLOG_INFO("Initializing Vulcanite Engine");

  core::Vfs::Providers providers;
  auto& assetDir = DataDirectory.value();
  SPDLOG_INFO("Using asset directory {}", assetDir);
  providers.push_back(
      std::make_unique<core::Vfs::FilesystemProvider>(assetDir));
  mVfs = std::make_unique<core::Vfs>(std::move(providers));

  mPipeline = std::make_unique<VulkanRenderPipeline>(handle, mWindow,
                                                     mThreadPool, *mVfs);

  initEcs();

  mUi.push_back(std::make_unique<ui::CvarUi>(core::Cvar::get()));
  mUi.push_back(std::make_unique<ui::ProfilerUi>(mProfiler));
  mUi.push_back(std::make_unique<ui::SceneViewUi>(mEcs));
  mUi.push_back(std::make_unique<ui::UsageUi>(assets::INativeHandleProvider::get()));

  SPDLOG_INFO("Ready to go!");
}

void VulkanEngine::initEcs() {
  auto& handles = assets::INativeHandleProvider::get();

  // Allocate an image to fill the window
  auto cameraobj = mEcs.createEntity();
  mEcs.addComponent(cameraobj, ecs::Transform{
                                   .mTranslation = glm::vec3(0.0f, 0.0f, 3.0f),
                               });
  auto draw = mPipeline->createDrawImage(mWindow.getSize());
  mEcs.addComponent(cameraobj,
                    ecs::Camera{
                        .mType = ecs::Camera::ProjectionType::Perspective,
                        .mNear = 0.1f,
                        .mFar = 10000.0f,
                        .mFov = glm::radians(70.0f),
                        .mSize = mWindow.getSize(),
                        .mImages = draw,
                    });
  // Camera owns its images
  handles.decRef(draw.draw);
  handles.decRef(draw.depth);

  mCamera = mEcs.addSystem(std::make_unique<ecs::CameraSystem>(
      cameraobj, mWindow.getKeyboard(), mWindow));
  // mEcs.addSystem(std::make_unique<ecs::CameraPathSystem>(
  //     cameraobj, mVfs->get("paths/default.json")));
  mEcs.addCommandBarrier();
  mEcs.addSystem(mPipeline->createRenderSystem());

  auto mesh = assets::MeshLoader::loadGltf(
      mVfs->get("meshes/third_party/structure.glb"));
  ecs::util::MeshInst::instantiate(mEcs, mesh, ecs::Transform{});
}

VulkanEngine::~VulkanEngine() {
  SPDLOG_INFO("Vulcanite shutting down. Goodbye!");

  // Pipeline must be idle before we can destory draw images
  mPipeline->waitIdle();
}

void VulkanEngine::run() {
  auto frameStart = std::chrono::steady_clock::now();
  while (!mWindow.quitRequested() &&
         (QuitAfterFrames.value() < 0 ||
          mPipeline->getFrameNumber() < QuitAfterFrames.value())) {
    auto now = std::chrono::steady_clock::now();
    core::Duration dt;
    if (FixedTimestep.value() > 0)
      dt = core::seconds(FixedTimestep.value());
    else
      dt = now - frameStart;
    frameStart = now;

    mProfiler.beginFrame();
    mWindow.update();

    if (mWindow.resized()) {
      auto data = mEcs.getComponent<ecs::Camera>(mCamera->getCamera());
      data.mImages = mPipeline->createDrawImage(mWindow.getSize());
      data.mSize = mWindow.getSize();

      // TODO: Do we need to waitIdle to swap out draw image?
      mEcs.executeImmediate(ecs::Camera::SetData{
          .mTarget = mCamera->getCamera(),
          .mData = data,
      });
    }

    ImGui::NewFrame();

    mProfiler.pushSection("Thread Sync");
    mThreadPool.finalise();

    mProfiler.siblingSection("Input");

    mProfiler.siblingSection("GUI");
    ImGui_ImplVulkan_NewFrame();

    if (mWindow.getKeyboard().getDigital(
            sdl::Keyboard::DigitalControl::ToggleConsole))
      mConsoleVisible = !mConsoleVisible;

    if (mConsoleVisible && ImGui::BeginMainMenuBar()) {
      if (ImGui::BeginMenu("UI")) {
        for (auto& element : mUi) {
          ImGui::MenuItem(element->name(), /*shortcut=*/nullptr,
                          element->visibleRef());
        }
        ImGui::EndMenu();
      }
      ImGui::EndMainMenuBar();
    }

    for (auto& element : mUi) {
      element->draw();
    }

    ImGui::Render();

    // TODO: Key to show ImGUI demo?
    mPipeline->beginFrame();

    mProfiler.siblingSection("ECS");
    mEcs.update(dt);

    mProfiler.siblingSection("Present Frame");
    mPipeline->present(mEcs.getComponent<ecs::Camera>(mCamera->getCamera()));
    mProfiler.popSection();
    mProfiler.endFrame();
  }

  mThreadPool.awaitAll();
  mThreadPool.finalise();
}

} // namespace selwonk::vulkan
