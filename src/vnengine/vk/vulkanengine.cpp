#include "vulkanengine.hpp"

// #include "../ecs/camerapathsystem.hpp"
#include "glm/gtc/quaternion.hpp"
#include "rendersystem.hpp"
#include "vnassets/debug.hpp"
#include "vncore/profiler.hpp"
#include "vncore/vfs.hpp"
#include "vnecs/named.hpp"
#include "vnecs/renderable.hpp"
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

// TODO: Set based on CPU count
// TODO: Unsigned flag
core::Cvar::Int WorkerThreads(
    "core.worker_threads", 8,
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

  mPipeline = std::make_unique<VulkanRenderPipeline>(handle, * mVfs);

  // No more VkBootstrap - you're on your own now.
  mImgui.init(handle, mWindow.getSdl());


  initEcs();
  writeBackgroundDescriptors();

  mCvarUi = std::make_unique<ui::CvarUi>(core::Cvar::get());

  // Changing descriptor array sizes will dirty pipelines
  auto dirtyBuffers = [this](int _) { mPipelinesDirty = true; };
  VulkanNativeHandleProvider::MaxVertexBuffers.getStore().addChange(
    dirtyBuffers);
  VulkanNativeHandleProvider::MaxTextures.getStore().addChange(dirtyBuffers);

  SPDLOG_INFO("Ready to go!");
}

void VulkanEngine::initEcs() {
  // Allocate an image to fill the window
  auto draw = initDrawImage(mWindow.getSize());
  auto cameraobj = mEcs.createEntity();
  mEcs.addComponent(cameraobj, ecs::Transform{
                                   .mTranslation = glm::vec3(0.0f, 0.0f, 3.0f),
                               });
  mEcs.addComponent(cameraobj,
                    ecs::Camera{
                        .mType = ecs::Camera::ProjectionType::Perspective,
                        .mNear = 0.1f,
                        .mFar = 10000.0f,
                        .mFov = glm::radians(70.0f),
                        .mSize = mWindow.getSize(),
                        .mDraw = draw.draw,
                        .mDepth = draw.depth,
                    });

  mCamera = mEcs.addSystem(std::make_unique<ecs::CameraSystem>(
      cameraobj, mWindow.getKeyboard(), mWindow));
  // mEcs.addSystem(std::make_unique<ecs::CameraPathSystem>(
  //     cameraobj, mVfs->get("paths/default.json")));
  mEcs.addCommandBarrier();
  mEcs.addSystem(std::make_unique<RenderSystem>(*this));

  auto mesh = assets::MeshLoader::loadGltf(
      mVfs->get("meshes/third_party/structure.glb"));
  ecs::util::MeshInst::instantiate(mEcs, mesh, ecs::Transform{});
}

VulkanEngine::~VulkanEngine() {
  SPDLOG_INFO("Vulcanite shutting down. Goodbye!");

  // Let the GPU finish its work
  CHECK(mPipeline->mHandle.mDevice.waitIdle());

  mImgui.destroy(mPipeline->mHandle);

  auto& camera = mEcs.getComponent<ecs::Camera>(mCamera->getCamera());
  // TODO: Do this in the camera
  mNativeHandles.getNativeTextures().decRef(camera.mDraw);
  mNativeHandles.getNativeTextures().decRef(camera.mDepth);
}

void VulkanEngine::writeBackgroundDescriptors() {
  // TODO: The camera should hold post-processing descriptors
  auto& camera = mEcs.getComponent<ecs::Camera>(mCamera->getCamera());
  auto& draw = mNativeHandles.getNativeTextures().getTexture(camera.mDraw);
  DescriptorAllocator::writeImage(mPipeline->mDrawImageDescriptors, draw.getView(), 0,
                                  vk::ImageLayout::eGeneral,
                                  vk::DescriptorType::eStorageImage);
}

VulkanEngine::CameraImages VulkanEngine::initDrawImage(glm::uvec2 size) {
  vk::ImageUsageFlags drawImageUsage = vk::ImageUsageFlagBits::eTransferSrc |
                                       vk::ImageUsageFlagBits::eTransferDst |
                                       vk::ImageUsageFlagBits::eStorage |
                                       vk::ImageUsageFlagBits::eColorAttachment;

  vk::Extent3D drawExtent = {size.x, size.y, 1};

  Image draw;
  draw.allocate(drawExtent, DrawFormat, drawImageUsage, "ImgDraw");
  Image depth;
  depth.allocate(drawExtent, DepthFormat,
                 vk::ImageUsageFlagBits::eDepthStencilAttachment, "ImgDepth");

  return {
      .draw = mNativeHandles.getNativeTextures().insert(draw),
      .depth = mNativeHandles.getNativeTextures().insert(depth),
  };
}

void VulkanEngine::initPipelines() {
  mPipelinesDirty = false;
  ShaderStage triangleStage(mVfs->get("shaders/triangle.vert.spv"),
                            vk::ShaderStageFlags::BitsType::eVertex, "main");
  ShaderStage fragmentStage(mVfs->get("shaders/triangle.frag.spv"),
                            vk::ShaderStageFlags::BitsType::eFragment, "main");
  auto layouts = getDescriptorLayouts();
  auto builder = Pipeline::Builder();
  builder.setShaders(triangleStage, fragmentStage)
      .setInputTopology(vk::PrimitiveTopology::eTriangleList)
      .setPolygonMode(vk::PolygonMode::eFill)
      .setCullMode(vk::CullModeFlagBits::eBack,
                   vk::FrontFace::eCounterClockwise)
      .disableMultisampling()
      .disableBlending()
      .setDescriptorLayouts(std::span(layouts))
      .enableDepth(true, vk::CompareOp::eGreaterOrEqual)
      .setDepthFormat(DepthFormat)
      .setColorAttachFormat(DrawFormat);

  mOpaquePipeline = builder.build(mPipeline->mHandle.mDevice);
  mTranslucentPipeline = builder
                             // Disable depth write
                             .enableDepth(false, vk::CompareOp::eGreaterOrEqual)
                             .enableAlphaBlend()
                             .build(mPipeline->mHandle.mDevice);
}

void VulkanEngine::run() {
  auto frameStart = std::chrono::steady_clock::now();
  while (!mWindow.quitRequested() && (QuitAfterFrames.value() < 0 ||
                                      mFrameNumber < QuitAfterFrames.value())) {
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
      auto draw = initDrawImage(mWindow.getSize());
      auto data = mEcs.getComponent<ecs::Camera>(mCamera->getCamera());
      // TODO: Do this in the camera
      mNativeHandles.getNativeTextures().decRef(data.mDraw);
      mNativeHandles.getNativeTextures().decRef(data.mDepth);
      data.mDraw = draw.draw;
      data.mDepth = draw.depth;
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
                       mNativeHandles.getNativeTextures().size(),
                       mNativeHandles.getNativeTextures().getCapacity());
      ImGui::LabelText("Samplers", "%i/%i",
                       mNativeHandles.getNativeSamplers().size(),
                       mNativeHandles.getNativeSamplers().capacity());
      ImGui::LabelText("Vertex Buffers", "%i/%i",
                       mNativeHandles.getNativeVertexes().size(),
                       mNativeHandles.getNativeVertexes().getCapacity());
      ImGui::LabelText("Index Buffers", "%i/%i",
                       mNativeHandles.getNativeIndexes().size(),
                       mNativeHandles.getNativeIndexes().getCapacity());
      ImGui::LabelText("Materials", "%i/%i",
                       mNativeHandles.getNativeMaterials().size(),
                       mNativeHandles.getNativeMaterials().capacity());

      auto& frameData = getCurrentFrame();
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
        ImGui::TableSetupColumn("Scale");
        ImGui::TableSetupColumn("Rotation");
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
    if (mPipelinesDirty) {
      // Recreate pipelines on the first frame or when a descriptor's cvar
      // changes
      // TODO: Render provider should own this and create our render system
      initPipelines();
      VulkanDebugRenderer::get().initPipelines();
    }

    mProfiler.siblingSection("ECS");
    mEcs.update(dt);

    mProfiler.siblingSection("Present Frame");
    present();
    mProfiler.popSection();
    mProfiler.endFrame();
  }

  mThreadPool.awaitAll();
  mThreadPool.finalise();
}

VulkanRenderPipeline::FrameData& VulkanEngine::prepareRendering() {
  auto& frame = getCurrentFrame();
  auto cmd = frame.mCommandBuffer;

  // Wait for the previous frame to finish
  CHECK(VulkanHandle::get().mDevice.waitForFences(1, &frame.mRenderFence, true,
                                                  core::RenderTimeout));
  CHECK(VulkanHandle::get().mDevice.resetFences(1, &frame.mRenderFence));

  // We're certain the command buffer is not in use, prepare for recording
  CHECK(vkResetCommandBuffer(cmd, 0));
  // We won't be submitting the buffer multiple times in a row, let Vulkan know
  // Drivers may be able to get a small speed boost
  auto beginInfo = VulkanInit::commandBufferBeginInfo(
      vk::CommandBufferUsageFlags::BitsType::eOneTimeSubmit);
  CHECK(cmd.begin(&beginInfo));
  return frame;
}

void VulkanEngine::present() {
  auto& frame = getCurrentFrame();
  auto cmd = frame.mCommandBuffer;
  auto& camera = mEcs.getComponent<ecs::Camera>(mCamera->getCamera());

  // Request a buffer to draw to
  uint32_t swapchainImageIndex;
  CHECK(mPipeline->mHandle.mDevice.acquireNextImageKHR(
      mPipeline->mHandle.mSwapchain, core::RenderTimeout, frame.mSwapchainSemaphore,
      nullptr, &swapchainImageIndex));
  auto& swapchainEntry = mPipeline->mHandle.mSwapchainEntries[swapchainImageIndex];

  // Copy draw image to the swapchain
  Image::transition(cmd, swapchainEntry.image, vk::ImageLayout::eUndefined,
                    vk::ImageLayout::eTransferDstOptimal);
  Image::copyToSwapchainImage(
      cmd, mNativeHandles.getNativeTextures().getTexture(camera.mDraw),
      swapchainEntry.image, mPipeline->mHandle.mSwapchainExtent);

  Image::transition(cmd, swapchainEntry.image,
                    vk::ImageLayout::eTransferDstOptimal,
                    vk::ImageLayout::eAttachmentOptimal);
  // Draw directly to the swapchain, which matches the format ImGui expects
  mImgui.draw(mPipeline->mHandle, cmd, swapchainEntry.view);
  Image::transition(cmd, swapchainEntry.image,
                    vk::ImageLayout::eAttachmentOptimal,
                    vk::ImageLayout::ePresentSrcKHR);

  // Finalise the command buffer, ready for execution
  CHECK(cmd.end());

  // Submit, after all this time
  auto cmdInfo = VulkanInit::commandBufferSubmitInfo(cmd);
  auto waitInfo = VulkanInit::semaphoreSubmitInfo(
      frame.mSwapchainSemaphore,
      vk::PipelineStageFlags2::BitsType::eColorAttachmentOutput);
  auto signalInfo = VulkanInit::semaphoreSubmitInfo(
      swapchainEntry.semaphore,
      vk::PipelineStageFlags2::BitsType::eAllGraphics);
  auto submit = VulkanInit::submitInfo(&cmdInfo, &waitInfo, &signalInfo);
  // Execute
  CHECK(mPipeline->mHandle.mGraphicsQueue.submit2(1, &submit, frame.mRenderFence));

  vk::PresentInfoKHR presentInfo{.waitSemaphoreCount = 1,
                                 .pWaitSemaphores = &swapchainEntry.semaphore,
                                 .swapchainCount = 1,
                                 .pSwapchains = &mPipeline->mHandle.mSwapchain,
                                 .pImageIndices = &swapchainImageIndex};
  auto result = mPipeline->mHandle.mGraphicsQueue.presentKHR(&presentInfo);
  switch (result) {
  case vk::Result::eSuboptimalKHR:
  case vk::Result::eErrorOutOfDateKHR:
    // FIXME: Erroring elsewhere after a resize
    SPDLOG_ERROR("vkPresentKHR errored with {}, did the window resize?",
                 string_VkResult(static_cast<VkResult>(result)));
    break;
  case vk::Result::eSuccess:
    break;
  default:
    CHECK(result); // Fail with error
  }
  mFrameNumber++;
}

} // namespace selwonk::vulkan
