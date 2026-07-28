#include "vulkanengine.hpp"

// #include "../ecs/camerapathsystem.hpp"
#include "buffer.hpp"
#include "image.hpp"
#include "meshloader.hpp"
#include "rendersystem.hpp"
#include "shader.hpp"
#include "utility.hpp"
#include "vncore/profiler.hpp"
#include "vncore/vfs.hpp"
#include "vulkan/vulkan.hpp"
#include "vulkanhandle.hpp"
#include "vulkaninit.hpp"
#include <vncore/cvar.hpp>
#include <vncore/platform.hpp>
#include <vncore/times.hpp>

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

core::Cvar::Int MaxVertexBuffers("render.max_vertex_buffers", 8192,
                                 "Maximum number of vertex buffers",
                                 core::Cvar::Flags::Unsigned);

core::Cvar::Int MaxMaterials("render.max_materials", 8192,
                             "Maximum number of materials",
                             core::Cvar::Flags::Unsigned);

core::Cvar::Int
    MaxFrameInstances("render.max_frame_instances", 64 * 1024,
                      "Maximum number of instances per frame",
                      core::util::combineFlags(core::Cvar::Flags::InitOnly,
                                               core::Cvar::Flags::Unsigned));

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
    : mThreadPool(WorkerThreads.value()), mWindow(window), mHandle(handle),
      mVertexBuffers(MaxVertexBuffers), mIndexBuffers(MaxVertexBuffers) {

  SPDLOG_INFO("Initializing Vulcanite Engine");

  // No more VkBootstrap - you're on your own now.
  mImgui.init(mHandle, mWindow.getSdl());

  core::Vfs::Providers providers;
  auto& assetDir = DataDirectory.value();
  SPDLOG_INFO("Using asset directory {}", assetDir);
  providers.push_back(
      std::make_unique<core::Vfs::FilesystemProvider>(assetDir));
  mVfs = std::make_unique<core::Vfs>(std::move(providers));

  initDescriptors();
  initCommands();
  initEcs();
  writeBackgroundDescriptors();

  mCvarUi = std::make_unique<core::CvarUi>(core::Cvar::get());

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

  mCamera = mEcs.addSystem(std::make_unique<CameraSystem>(
      cameraobj, mWindow.getKeyboard(), mWindow));
  // mEcs.addSystem(std::make_unique<ecs::CameraPathSystem>(
  //     cameraobj, mVfs->get("paths/default.json")));
  mEcs.addCommandBarrier();
  mEcs.addSystem(std::make_unique<RenderSystem>(*this));

  auto mesh =
      MeshLoader::loadGltf(mVfs->get("meshes/third_party/structure.glb"));
  mesh->instantiate(mEcs, ecs::Transform{});
}

VulkanEngine::~VulkanEngine() {
  SPDLOG_INFO("Vulcanite shutting down. Goodbye!");

  // Let the GPU finish its work
  CHECK(mHandle.mDevice.waitIdle());
  for (auto& frameData : mFrameData) {
    frameData.destroy(mHandle, *this);
  }
  mImgui.destroy(mHandle);

  auto& camera = mEcs.getComponent<ecs::Camera>(mCamera->getCamera());
  // TODO: Do this in the camera
  mNativeHandles.getNativeTextures().decRef(camera.mDraw);
  mNativeHandles.getNativeTextures().decRef(camera.mDepth);

  mGradientShader.free();
  mGlobalDescriptorAllocator.destroy();
  // This will also destroy all descriptor sets allocated by it
  mHandle.mDevice.destroyDescriptorSetLayout(mDrawImageDescriptorLayout,
                                             nullptr);
  mHandle.mDevice.destroyDescriptorSetLayout(mSceneUniformDescriptorLayout,
                                             nullptr);
  mHandle.mDevice.destroyDescriptorSetLayout(mInstanceDataLayout, nullptr);

  mNativeHandles.getNativeMaterials().decRef(mDefaultMaterial.mDataIndex);
}

void VulkanEngine::writeBackgroundDescriptors() {
  // TODO: The camera should hold post-processing descriptors
  auto& camera = mEcs.getComponent<ecs::Camera>(mCamera->getCamera());
  auto& draw = mNativeHandles.getNativeTextures().getTexture(camera.mDraw);
  DescriptorAllocator::writeImage(mDrawImageDescriptors, draw.getView(), 0,
                                  vk::ImageLayout::eGeneral,
                                  vk::DescriptorType::eStorageImage);
}

void VulkanEngine::FrameData::init(VulkanHandle& handle, VulkanEngine& engine) {
  auto poolInfo =
      VulkanInit::commandPoolCreateInfo(handle.mGraphicsQueueFamily);

  // Allocate a pool that will allocate buffers
  CHECK(handle.mDevice.createCommandPool(&poolInfo, nullptr, &mCommandPool));

  // Allocate a default command buffer to submit into
  auto allocInfo = VulkanInit::bufferAllocateInfo(mCommandPool);
  CHECK(handle.mDevice.allocateCommandBuffers(&allocInfo, &mCommandBuffer));

  mSwapchainSemaphore = handle.createSemaphore();

  // Create the fence in the "signalled" state so we can wait on it immediately
  // Simplifies first-frame logic
  mRenderFence = handle.createFence(/*signalled=*/true);

  mSceneUniforms.allocate(handle.mAllocator);
  mSceneUniformDescriptor = engine.mGlobalDescriptorAllocator.allocate(
      engine.mSceneUniformDescriptorLayout);
  DescriptorAllocator::writeBuffer(mSceneUniformDescriptor,
                                   vk::DescriptorType::eUniformBuffer,
                                   mSceneUniforms.getBuffer().getBuffer(),
                                   /*offset=*/0);

  mFrameDataBuffer.allocate(MaxFrameInstances.value() *
                                sizeof(interop::VertexInstanceData),
                            Buffer::Usage::FrameData);
  mFrameData =
      core::BumpAllocator(mFrameDataBuffer.getAllocationInfo().pMappedData,
                          mFrameDataBuffer.getSize());

  mInstanceDataDescriptor =
      engine.mGlobalDescriptorAllocator.allocate(engine.mInstanceDataLayout);
  DescriptorAllocator::writeBuffer(mInstanceDataDescriptor,
                                   vk::DescriptorType::eStorageBuffer,
                                   mFrameDataBuffer.getBuffer(),
                                   /*offset=*/0); // TODO: Add static size

  interop::SceneData* data = mSceneUniforms.data();
  data->sunDirection = glm::vec3(0, 1.0f, 0.5f);
  data->sunColor = glm::vec3(1.0f, 1.0f, 1.0f);
  data->ambientColor = glm::vec3(0.1f, 0.1f, 0.1f);
}

void VulkanEngine::FrameData::destroy(VulkanHandle& handle,
                                      VulkanEngine& engine) {
  // Destroying a queue will destroy all its buffers
  handle.mDevice.destroyCommandPool(mCommandPool, nullptr);
  handle.destroySemaphore(mSwapchainSemaphore);
  handle.destroyFence(mRenderFence);
  mSceneUniforms.free(handle.mAllocator);
  mFrameDataBuffer.free(handle.mAllocator);
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

void VulkanEngine::initCommands() {
  SPDLOG_INFO("Initialising command buffers");

  for (auto& buffer : mFrameData) {
    buffer.init(mHandle, *this);
  }
}

void VulkanEngine::initDescriptors() {
  // Allocate a descriptor pool to hold images that compute shaders may write to
  std::array<DescriptorAllocator::PoolSizeRatio, 4> sizes = {{
      {vk::DescriptorType::eStorageImage, 1},
      {vk::DescriptorType::eUniformBuffer, 1},
      {vk::DescriptorType::eStorageBuffer,
       static_cast<float>(MaxFrameInstances.value())},
      {vk::DescriptorType::eSampledImage, 1},
  }};

  // Reserve space for 10 such descriptors
  mGlobalDescriptorAllocator.init(10, sizes);

  // Allocate one of these descriptors
  DescriptorLayoutBuilder computeDescBuilder;
  computeDescBuilder.addBinding(0, vk::DescriptorType::eStorageImage);
  mDrawImageDescriptorLayout = computeDescBuilder.build(
      mHandle.mDevice, vk::ShaderStageFlags::BitsType::eCompute);
  mDrawImageDescriptors =
      mGlobalDescriptorAllocator.allocate(mDrawImageDescriptorLayout);

  DescriptorLayoutBuilder uniformBuilder;
  uniformBuilder.addBinding(0, vk::DescriptorType::eUniformBuffer);
  mSceneUniformDescriptorLayout = uniformBuilder.build(
      mHandle.mDevice,
      vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment);

  DescriptorLayoutBuilder sceneDataBuilder;
  sceneDataBuilder.addBinding(0, vk::DescriptorType::eStorageBuffer,
                              MaxFrameInstances.value());
  mInstanceDataLayout = sceneDataBuilder.build(
      mHandle.mDevice,
      vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment);

  ShaderStage stage(mVfs->get("shaders/gradient.comp.spv"),
                    vk::ShaderStageFlags::BitsType::eCompute, "main");
  mGradientShader.link(mDrawImageDescriptorLayout, stage,
                       sizeof(interop::GradientPushConstants));

  DescriptorLayoutBuilder bindlessBuilder;
  mNativeHandles.getNativeMaterials().init(MaxMaterials);

  mDebug = std::make_unique<Debug>();

  // Changing descriptor array sizes will dirty pipelines
  auto dirtyBuffers = [this](int _) { mPipelinesDirty = true; };
  MaxVertexBuffers.getStore().addChange(dirtyBuffers);
  VulkanNativeHandleProvider::MaxTextures.getStore().addChange(dirtyBuffers);

  interop::MaterialData defaultMat = {
      .colorFactors = glm::vec4(1.0f),
      .metalRoughnessFactors = glm::vec4(1.0f),
  };

  mDefaultMaterial = assets::Material{
      .mTexture = mNativeHandles.getNativeTextures().getMissing(),
      .mDataIndex = mNativeHandles.getNativeMaterials().insert(defaultMat),
      .mSampler = mNativeHandles.getSampler({
          .mMinFilter = fastgltf::Filter::Nearest,
          .mMagFilter = fastgltf::Filter::Nearest,
      }),
      .mPass = assets::Material::Pass::Opaque,
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

  mOpaquePipeline = builder.build(mHandle.mDevice);
  mTranslucentPipeline = builder
                             // Disable depth write
                             .enableDepth(false, vk::CompareOp::eGreaterOrEqual)
                             .enableAlphaBlend()
                             .build(mHandle.mDevice);
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
      mHandle.resizeSwapchain(mWindow.getSize());
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

    mProfiler.printTimes();

    if (ImGui::Begin("Limits & Usage")) {
      ImGui::LabelText("Textures", "%zu/%i",
                       mNativeHandles.getNativeTextures().size(),
                       mNativeHandles.getNativeTextures().getCapacity());
      ImGui::LabelText("Samplers", "%i/%i",
                       mNativeHandles.getNativeSamplers().size(),
                       mNativeHandles.getNativeSamplers().capacity());
      ImGui::LabelText("Vertex Buffers", "%i/%i", mVertexBuffers.size(),
                       mVertexBuffers.getCapacity());
      ImGui::LabelText("Index Buffers", "%i/%i", mIndexBuffers.size(),
                       mIndexBuffers.getCapacity());
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

    ImGui::Render();

    mProfiler.siblingSection("Load Shaders");
    // Changing a CVAR may invalidate pipelines, so we must check after GUI
    // update
    if (mPipelinesDirty) {
      // Recreate pipelines on the first frame or when a descriptor's cvar
      // changes
      initPipelines();
      mDebug->initPipelines();
    }

    mProfiler.siblingSection("ECS");
    mEcs.update(dt);

    mProfiler.siblingSection("Present Frame");
    present();
    mProfiler.popSection();
    mProfiler.endFrame();
  }
}

VulkanEngine::FrameData& VulkanEngine::prepareRendering() {
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
  CHECK(mHandle.mDevice.acquireNextImageKHR(
      mHandle.mSwapchain, core::RenderTimeout, frame.mSwapchainSemaphore,
      nullptr, &swapchainImageIndex));
  auto& swapchainEntry = mHandle.mSwapchainEntries[swapchainImageIndex];

  // Copy draw image to the swapchain
  Image::transition(cmd, swapchainEntry.image, vk::ImageLayout::eUndefined,
                    vk::ImageLayout::eTransferDstOptimal);
  Image::copyToSwapchainImage(
      cmd, mNativeHandles.getNativeTextures().getTexture(camera.mDraw),
      swapchainEntry.image, mHandle.mSwapchainExtent);

  Image::transition(cmd, swapchainEntry.image,
                    vk::ImageLayout::eTransferDstOptimal,
                    vk::ImageLayout::eAttachmentOptimal);
  // Draw directly to the swapchain, which matches the format ImGui expects
  mImgui.draw(mHandle, cmd, swapchainEntry.view);
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
  CHECK(mHandle.mGraphicsQueue.submit2(1, &submit, frame.mRenderFence));

  vk::PresentInfoKHR presentInfo{.waitSemaphoreCount = 1,
                                 .pWaitSemaphores = &swapchainEntry.semaphore,
                                 .swapchainCount = 1,
                                 .pSwapchains = &mHandle.mSwapchain,
                                 .pImageIndices = &swapchainImageIndex};
  auto result = mHandle.mGraphicsQueue.presentKHR(&presentInfo);
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
