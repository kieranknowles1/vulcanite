#include "vulkanengine.hpp"

#include "../core/cvar.hpp"
#include "../platform.hpp"
#include "../times.hpp"
#include "imagehelpers.hpp"
#include "material.hpp"
#include "meshloader.hpp"
#include "rendersystem.hpp"
#include "samplercache.hpp"
#include "shader.hpp"
#include "utility.hpp"
#include "vulkan/vulkan.hpp"
#include "vulkanhandle.hpp"
#include "vulkaninit.hpp"

#include <cassert>
#include <cmath>
#include <cstdint>

#include <VkBootstrap.h>
#include <fmt/base.h>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/packing.hpp>
#include <glm/trigonometric.hpp>
#include <imgui.h>
#include <imgui_impl_vulkan.h>
#include <memory>
#include <vulkan/vk_enum_string_helper.h>

namespace selwonk::vulkan {

core::Cvar::Int MaxVertexBuffers("render.max_vertex_buffers", 8192,
                                 "Maximum number of vertex buffers");
core::Cvar::Int MaxSamplers("render.max_samplers", 32,
                            "Maximum number of samplers");
core::Cvar::Int MaxTextures("render.max_textures", 8192,
                            "Maximum number of textures");

VulkanEngine::VulkanEngine(const core::Cli& cli, core::Settings& settings,
                           core::Window& window, VulkanHandle& handle)
    : mCli(cli), mSettings(settings), mWindow(window), mHandle(handle),
      mSamplerCache(MaxSamplers), mTextureManager(MaxTextures) {

  fmt::println("Initializing Vulcanite Engine");

  mSettings = settings;

  // No more VkBootstrap - you're on your own now.
  mImgui.init(mHandle, mWindow.getSdl());

  Vfs::Providers providers;
  auto assetDir = Platform::getExePath().parent_path() / "assets";
  fmt::println("Using asset directory {}", assetDir.c_str());
  providers.push_back(std::make_unique<Vfs::FilesystemProvider>(assetDir));
  mVfs = std::make_unique<Vfs>(std::move(providers));

  initDescriptors();
  initCommands();
  initEcs();
  writeBackgroundDescriptors();

  fmt::println("Ready to go!");
}

void VulkanEngine::initEcs() {
  // Allocate an image to fill the window
  auto draw = initDrawImage(mSettings.initialSize);
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
                        .mDrawTarget = draw.draw,
                        .mDepthTarget = draw.depth,
                    });

  mCamera = mEcs.addSystem(std::make_unique<CameraSystem>(
      cameraobj, mWindow.getKeyboard(), mWindow));
  mEcs.addSystem(std::make_unique<RenderSystem>(*this));

  mMesh = MeshLoader::loadGltf("third_party/structure.glb");
  mMesh->instantiate(mEcs, ecs::Transform{});
}

VulkanEngine::~VulkanEngine() {
  fmt::println("Vulcanite shutting down. Goodbye!");

  // Let the GPU finish its work
  vkDeviceWaitIdle(mHandle.mDevice);
  for (auto& frameData : mFrameData) {
    frameData.destroy(mHandle, *this);
  }
  mImgui.destroy(mHandle);

  mGradientShader.free();
  mGlobalDescriptorAllocator.destroy();
  // This will also destroy all descriptor sets allocated by it
  mHandle.mDevice.destroyDescriptorSetLayout(mDrawImageDescriptorLayout,
                                             nullptr);
  mHandle.mDevice.destroyDescriptorSetLayout(mSceneUniformDescriptorLayout,
                                             nullptr);
  mDefaultMaterialData.free(mHandle.mAllocator);
}

void VulkanEngine::writeBackgroundDescriptors() {
  // TODO: The camera should hold post-processing descriptors
  auto& camera = mEcs.getComponent<ecs::Camera>(mCamera->getCamera());
  mDrawImageDescriptors.write(mHandle.mDevice,
                              {
                                  camera.mDrawTarget->getView(),
                                  vk::DescriptorType::eStorageImage,
                                  vk::ImageLayout::eGeneral,
                              });
}

void VulkanEngine::FrameData::init(VulkanHandle& handle, VulkanEngine& engine) {
  auto poolInfo =
      VulkanInit::commandPoolCreateInfo(handle.mGraphicsQueueFamily);

  // Allocate a pool that will allocate buffers
  check(handle.mDevice.createCommandPool(&poolInfo, nullptr, &mCommandPool));

  // Allocate a default command buffer to submit into
  auto allocInfo = VulkanInit::bufferAllocateInfo(mCommandPool);
  check(handle.mDevice.allocateCommandBuffers(&allocInfo, &mCommandBuffer));

  mSwapchainSemaphore = handle.createSemaphore();

  // Create the fence in the "signalled" state so we can wait on it immediately
  // Simplifies first-frame logic
  mRenderFence = handle.createFence(/*signalled=*/true);

  mSceneUniforms.allocate(handle.mAllocator);
  mSceneUniformDescriptor = engine.mGlobalDescriptorAllocator
                                .allocate<StructBuffer<interop::SceneData>>(
                                    engine.mSceneUniformDescriptorLayout);
  mSceneUniformDescriptor.write(handle.mDevice, mSceneUniforms);

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
}

VulkanEngine::CameraImages VulkanEngine::initDrawImage(glm::uvec2 size) {
  vk::ImageUsageFlags drawImageUsage = vk::ImageUsageFlagBits::eTransferSrc |
                                       vk::ImageUsageFlagBits::eTransferDst |
                                       vk::ImageUsageFlagBits::eStorage |
                                       vk::ImageUsageFlagBits::eColorAttachment;

  vk::Extent3D drawExtent = {size.x, size.y, 1};
  return {
      .draw = std::make_shared<Image>(drawExtent, DrawFormat, drawImageUsage),
      .depth = std::make_shared<Image>(
          drawExtent, DepthFormat,
          vk::ImageUsageFlagBits::eDepthStencilAttachment),
  };
}

void VulkanEngine::initCommands() {
  fmt::println("Initialising command buffers");

  for (auto& buffer : mFrameData) {
    buffer.init(mHandle, *this);
  }
}

void VulkanEngine::initDescriptors() {
  // Allocate a descriptor pool to hold images that compute shaders may write to
  std::array<DescriptorAllocator::PoolSizeRatio, 4> sizes = {{
      {vk::DescriptorType::eStorageImage, 1},
      {vk::DescriptorType::eUniformBuffer, 1},
      {vk::DescriptorType::eStorageBuffer, 1},
      {vk::DescriptorType::eSampledImage, 1},
  }};

  // Reserve space for 10 such descriptors
  mGlobalDescriptorAllocator.init(10, sizes);

  // Allocate one of these descriptors
  DescriptorLayoutBuilder computeDescBuilder;
  computeDescBuilder.addBinding(0, vk::DescriptorType::eStorageImage);
  mDrawImageDescriptorLayout = computeDescBuilder.build(
      mHandle.mDevice, vk::ShaderStageFlags::BitsType::eCompute);
  mDrawImageDescriptors = mGlobalDescriptorAllocator.allocate<ImageDescriptor>(
      mDrawImageDescriptorLayout);

  DescriptorLayoutBuilder uniformBuilder;
  uniformBuilder.addBinding(0, vk::DescriptorType::eUniformBuffer);
  mSceneUniformDescriptorLayout = uniformBuilder.build(
      mHandle.mDevice,
      vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment);

  ShaderStage stage("gradient.comp.spv",
                    vk::ShaderStageFlags::BitsType::eCompute, "main");
  mGradientShader.link(mDrawImageDescriptorLayout, stage,
                       sizeof(interop::GradientPushConstants));

  DescriptorLayoutBuilder bindlessBuilder;
  mVertexBuffers.init(MaxVertexBuffers);
  mIndexBuffers.init(MaxVertexBuffers);

  mDebug = std::make_unique<Debug>();

  // Changing descriptor array sizes will dirty pipelines
  auto dirtyBuffers = [this](int _) { mPipelinesDirty = true; };
  MaxVertexBuffers.addChangeCallback(dirtyBuffers);
  MaxSamplers.addChangeCallback(dirtyBuffers);
  MaxTextures.addChangeCallback(dirtyBuffers);

  // TODO: Dedicated class for managing materials
  mDefaultMaterialData.allocate(mHandle.mAllocator,
                                vk::BufferUsageFlagBits::eShaderDeviceAddress);
  *mDefaultMaterialData.data() = {
      .colorFactors = glm::vec4(1.0f),
      .metalRoughnessFactors = glm::vec4(1.0f),
  };

  mDefaultMaterial = std::make_shared<Material>(Material{
      .mPipeline = &mOpaquePipeline,
      .mTexture = mTextureManager.getMissing(),
      .mData = mDefaultMaterialData.getDeviceAddress(),
      .mSampler = mSamplerCache.get({
          .magFilter = vk::Filter::eNearest,
          .minFilter = vk::Filter::eNearest,
      }),
      .mPass = Material::Pass::Opaque,
  });
}

void VulkanEngine::initPipelines() {
  mPipelinesDirty = false;
  ShaderStage triangleStage("triangle.vert.spv",
                            vk::ShaderStageFlags::BitsType::eVertex, "main");
  ShaderStage fragmentStage("triangle.frag.spv",
                            vk::ShaderStageFlags::BitsType::eFragment, "main");
  auto layouts = getDescriptorLayouts();
  auto builder =
      Pipeline::Builder()
          .setShaders(triangleStage, fragmentStage)
          .setInputTopology(vk::PrimitiveTopology::eTriangleList)
          .setPolygonMode(vk::PolygonMode::eFill)
          .setCullMode(vk::CullModeFlagBits::eBack,
                       vk::FrontFace::eCounterClockwise)
          .setPushConstantSize(vk::ShaderStageFlagBits::eVertex |
                                   vk::ShaderStageFlagBits::eFragment,
                               sizeof(interop::VertexPushConstants))
          .disableMultisampling()
          .disableBlending()
          .setDescriptorLayouts(std::span(layouts))
          .enableDepth(true, vk::CompareOp::eGreaterOrEqual)
          .setDepthFormat(DepthFormat)
          .setColorAttachFormat(DrawFormat);

  mOpaquePipeline = builder.build(mHandle.mDevice);
  mTranslucentPipeline = builder.enableAlphaBlend().build(mHandle.mDevice);
}

void VulkanEngine::run() {
  while (!mWindow.quitRequested() && (!mCli.quitAfterFrames.has_value() ||
                                      mFrameNumber < mCli.quitAfterFrames)) {
    ImGui::NewFrame();
    mProfiler.beginFrame();
    mProfiler.startSection("Input");
    mWindow.update();
    ImGui_ImplVulkan_NewFrame();

    // TODO: Delta time
    float dt = 1.0f / 60.0f;

    core::Cvar::get().displayUi();

    if (ImGui::Begin("Background")) {
      ImGui::LabelText("Textures", "%zu/%i", mTextureManager.size(),
                       mTextureManager.getCapacity());
      ImGui::LabelText("Samplers", "%zu/%i", mSamplerCache.size(),
                       mSamplerCache.getCapacity());

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

    mProfiler.startSection("Prepare Render");
    if (mPipelinesDirty) {
      initPipelines();
      mDebug->initPipelines();
    }

    ImGui::End();
    mProfiler.printTimes();

    ImGui::Render();
    if (mWindow.resized()) {
      mHandle.resizeSwapchain(mWindow.getSize());
      auto draw = initDrawImage(mWindow.getSize());
      auto& camera = mEcs.getComponent<ecs::Camera>(mCamera->getCamera());
      camera.mDrawTarget = draw.draw;
      camera.mDepthTarget = draw.depth;
      writeBackgroundDescriptors();
    }
    mEcs.update(dt);
    present();
    mProfiler.endFrame();
  }
}

VulkanEngine::FrameData& VulkanEngine::prepareRendering() {
  auto& frame = getCurrentFrame();
  auto cmd = frame.mCommandBuffer;

  // Wait for the previous frame to finish
  core::Profiler::get().startSection("Await VSync");
  check(VulkanHandle::get().mDevice.waitForFences(1, &frame.mRenderFence, true,
                                                  RenderTimeout));
  check(VulkanHandle::get().mDevice.resetFences(1, &frame.mRenderFence));

  // We're certain the command buffer is not in use, prepare for recording
  check(vkResetCommandBuffer(cmd, 0));
  // We won't be submitting the buffer multiple times in a row, let Vulkan know
  // Drivers may be able to get a small speed boost
  auto beginInfo = VulkanInit::commandBufferBeginInfo(
      vk::CommandBufferUsageFlags::BitsType::eOneTimeSubmit);
  check(cmd.begin(&beginInfo));
  return frame;
}

void VulkanEngine::present() {
  auto& frame = getCurrentFrame();
  auto cmd = frame.mCommandBuffer;
  auto& camera = mEcs.getComponent<ecs::Camera>(mCamera->getCamera());

  // Request a buffer to draw to
  uint32_t swapchainImageIndex;
  check(vkAcquireNextImageKHR(mHandle.mDevice, mHandle.mSwapchain,
                              RenderTimeout, frame.mSwapchainSemaphore, nullptr,
                              &swapchainImageIndex));
  auto& swapchainEntry = mHandle.mSwapchainEntries[swapchainImageIndex];

  // Copy draw image to the swapchain
  ImageHelpers::transitionImage(cmd, swapchainEntry.image,
                                vk::ImageLayout::eUndefined,
                                vk::ImageLayout::eTransferDstOptimal);
  Image::copyToSwapchainImage(cmd, *camera.mDrawTarget, swapchainEntry.image,
                              mHandle.mSwapchainExtent);

  ImageHelpers::transitionImage(cmd, swapchainEntry.image,
                                vk::ImageLayout::eTransferDstOptimal,
                                vk::ImageLayout::eAttachmentOptimal);
  // Draw directly to the swapchain, which matches the format ImGui expects
  mImgui.draw(mHandle, cmd, swapchainEntry.view);
  ImageHelpers::transitionImage(cmd, swapchainEntry.image,
                                vk::ImageLayout::eAttachmentOptimal,
                                vk::ImageLayout::ePresentSrcKHR);

  // Finalise the command buffer, ready for execution
  check(vkEndCommandBuffer(cmd));

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
  check(mHandle.mGraphicsQueue.submit2(1, &submit, frame.mRenderFence));

  vk::PresentInfoKHR presentInfo{.waitSemaphoreCount = 1,
                                 .pWaitSemaphores = &swapchainEntry.semaphore,
                                 .swapchainCount = 1,
                                 .pSwapchains = &mHandle.mSwapchain,
                                 .pImageIndices = &swapchainImageIndex};
  auto result = mHandle.mGraphicsQueue.presentKHR(&presentInfo);
  switch (result) {
  case vk::Result::eSuboptimalKHR:
  case vk::Result::eErrorOutOfDateKHR:
    fmt::println("vkPresentKHR errored with {}, did the window resize?",
                 string_VkResult(static_cast<VkResult>(result)));
    break;
  case vk::Result::eSuccess:
    break;
  default:
    check(result); // Fail with error
  }
  mFrameNumber++;
}

} // namespace selwonk::vulkan
