#include "vulkanengine.hpp"

// #include "../ecs/camerapathsystem.hpp"
#include "glm/gtc/quaternion.hpp"
#include "rendersystem.hpp"
#include "vnassets/debug.hpp"
#include "vncore/profiler.hpp"
#include "vncore/vfs.hpp"
#include "vnecs/named.hpp"
#include "vnecs/transform.hpp"
#include "vulkan/vulkan.hpp"
#include <vnassets/meshloader.hpp>
#include <vncore/cvar.hpp>
#include <vncore/platform.hpp>
#include <vncore/times.hpp>
#include <vnecs/util/meshinst.hpp>
#include <vnvulkan/image.hpp>
#include <vnvulkan/shader.hpp>
#include <vnvulkan/utility.hpp>
#include <vnvulkan/vulkanhandle.hpp>
#include <vnvulkan/vulkaninit.hpp>

#include <chrono>
#include <cstdint>

#include <backends/imgui_impl_vulkan.h>
#include <glm/glm.hpp>
#include <glm/trigonometric.hpp>
#include <imgui.h>
#include <memory>
#include <spdlog/spdlog.h>
#include <vulkan/vk_enum_string_helper.h>
#include <vulkan/vulkan_core.h>

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
    "core.worker_threads", getDefaultThreadCount,
    "${cpu_thread_count}",
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
    : mThreadPool(WorkerThreads.value()), mWindow(window),
      mProfilerUi(mProfiler) {

  SPDLOG_INFO("Initializing Vulcanite Engine");

  core::Vfs::Providers providers;
  auto& assetDir = DataDirectory.value();
  SPDLOG_INFO("Using asset directory {}", assetDir);
  providers.push_back(
      std::make_unique<core::Vfs::FilesystemProvider>(assetDir));
  mVfs = std::make_unique<core::Vfs>(std::move(providers));

  mPipeline = std::make_unique<VulkanRenderPipeline>(handle, mWindow, mThreadPool, *mVfs);


  initEcs();
  writeBackgroundDescriptors();

  mCvarUi = std::make_unique<ui::CvarUi>(core::Cvar::get());

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
  mEcs.addSystem(std::make_unique<RenderSystem>(*mPipeline));

  auto mesh = assets::MeshLoader::loadGltf(
      mVfs->get("meshes/third_party/structure.glb"));
  ecs::util::MeshInst::instantiate(mEcs, mesh, ecs::Transform{});
}

VulkanEngine::~VulkanEngine() {
  SPDLOG_INFO("Vulcanite shutting down. Goodbye!");
}

void VulkanEngine::writeBackgroundDescriptors() {
  // TODO: The camera should hold post-processing settings
  auto& camera = mEcs.getComponent<ecs::Camera>(mCamera->getCamera());
  auto& draw = getNativeHandles().getNativeTextures().getTexture(camera.mImages.draw);
  DescriptorAllocator::writeImage(mPipeline->mDrawImageDescriptors, draw.getView(), 0,
                                  vk::ImageLayout::eGeneral,
                                  vk::DescriptorType::eStorageImage);
}

void VulkanEngine::run() {
  auto frameStart = std::chrono::steady_clock::now();
  while (!mWindow.quitRequested() && (QuitAfterFrames.value() < 0 ||
                                      mPipeline->mFrameNumber < QuitAfterFrames.value())) {
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
      mPipeline->mHandle.resizeSwapchain(mWindow.getSize());
      auto data = mEcs.getComponent<ecs::Camera>(mCamera->getCamera());
      data.mImages = mPipeline->createDrawImage(mWindow.getSize());
      data.mSize = mWindow.getSize();

      mEcs.executeImmediate(ecs::Camera::SetData{
          .mTarget = mCamera->getCamera(),
          .mData = data,
      });
      writeBackgroundDescriptors();
    }

    ImGui::NewFrame();

    mProfiler.pushSection("Thread Sync");
    mThreadPool.finalise();

    mProfiler.siblingSection("Input");

    if (mWindow.getKeyboard().getDigital(
            sdl::Keyboard::DigitalControl::ToggleConsole)) {
      mConsoleVisible = !mConsoleVisible;
    }

    mProfiler.siblingSection("GUI");
    ImGui_ImplVulkan_NewFrame();

    if (mConsoleVisible) {
      mCvarUi->displayUi();
    }

    mProfilerUi.printTimes();

    if (ImGui::Begin("Limits & Usage")) {
      ImGui::LabelText("Textures", "%zu/%i",
                       getNativeHandles().getNativeTextures().size(),
                       getNativeHandles().getNativeTextures().getCapacity());
      ImGui::LabelText("Samplers", "%i/%i",
                       getNativeHandles().getNativeSamplers().size(),
                       getNativeHandles().getNativeSamplers().capacity());
      ImGui::LabelText("Vertex Buffers", "%i/%i",
                       getNativeHandles().getNativeVertexes().size(),
                       getNativeHandles().getNativeVertexes().getCapacity());
      ImGui::LabelText("Index Buffers", "%i/%i",
                       getNativeHandles().getNativeIndexes().size(),
                       getNativeHandles().getNativeIndexes().getCapacity());
      ImGui::LabelText("Materials", "%i/%i",
                       getNativeHandles().getNativeMaterials().size(),
                       getNativeHandles().getNativeMaterials().capacity());

      auto& frameData = mPipeline->getCurrentFrame();
      ImGui::LabelText(
          "Frame Data", "%s/%s",
          core::util::formatFilesize(frameData.mFrameData.offset()).c_str(),
          core::util::formatFilesize(frameData.mFrameData.capacity()).c_str());

      size_t ram = core::Platform::getMemoryUsage();
      ImGui::LabelText("Memory", "%s", core::util::formatFilesize(ram).c_str());

#ifdef VN_LOGCOMPONENTSTATS
      std::apply(
          [](const auto&... componentArrays) {
            ((ImGui::LabelText(
                 componentArrays.getTypeName(), "Count: %zd, Capacity: %zd",
                 componentArrays.size(), componentArrays.capacity())),
             ...);
          },
          mEcs.getComponentArrays());
#endif
    }
    ImGui::End();

    if (ImGui::Begin("Scene Nodes")) {
      // TODO: Optional version of forEach
      // TODO: display everything in a tree, show parents
      // TODO: selection for debug draw
      // TODO: Virtual scroll. Need partial forEach
      if (ImGui::BeginTable("Scene", 4,
                            ImGuiTableFlags_RowBg | ImGuiTableFlags_Borders |
                                ImGuiTableFlags_SizingFixedFit)) {
        ImGui::TableSetupColumn("Name");
        ImGui::TableSetupColumn("Translation");
        ImGui::TableSetupColumn("Rotation");
        ImGui::TableSetupColumn("Scale");
        ImGui::TableHeadersRow();
        mEcs.forEach<const ecs::Named&, const ecs::Transform&>([&](auto entity,
                                                                   auto name,
                                                                   auto tfm) {
          ImGui::TableNextRow();
          ImGui::TableNextColumn();
          ImGui::Text("%s", name.mName.c_str());
          ImGui::TableNextColumn();
          // TODO: Reusable toString or something for transform
          ImGui::Text("%.1f,%.1f,%.1f", tfm.mTranslation.x, tfm.mTranslation.y, tfm.mTranslation.z);
          ImGui::TableNextColumn();
          auto euler = glm::degrees(glm::eulerAngles(tfm.mRotation));
          ImGui::Text("%.0f,%.0f,%.0f", euler.x, euler.y, euler.z);
          ImGui::TableNextColumn();
          ImGui::Text("%.2f,%.2f,%.2f", tfm.mScale.x, tfm.mScale.y, tfm.mScale.z);
        });
        ImGui::EndTable();
      }
    }
    ImGui::End();

    ImGui::Render();

    // TODO: Key to show ImGUI demo?

    mProfiler.siblingSection("Load Shaders");
    // Changing a CVAR may invalidate pipelines, so we must check after GUI
    // update
    if (mPipeline->mPipelinesDirty) {
      // Recreate pipelines on the first frame or when a descriptor's cvar
      // changes
      // TODO: Render provider should own this and create our render system
      // TODO: Move to render system?
      mPipeline->initPipelines();
      VulkanDebugRenderer::get().initPipelines();
    }

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
