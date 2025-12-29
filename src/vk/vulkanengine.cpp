#include "vulkanengine.hpp"

#include "../times.hpp"
#include "imagehelpers.hpp"
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
#include <glm/gtc/matrix_transform.hpp>
#include <glm/packing.hpp>
#include <glm/trigonometric.hpp>
#include <imgui.h>
#include <imgui_impl_vulkan.h>
#include <memory>

namespace selwonk::vulkan {
VulkanEngine *sEngineInstance = nullptr;
VulkanEngine &VulkanEngine::get() {
  assert(sEngineInstance != nullptr && "Engine instance not initialized");
  return *sEngineInstance;
}

VulkanEngine::VulkanEngine(core::Window &window) : mWindow(window) {}

VulkanEngine::~VulkanEngine() {
  assert(sEngineInstance != this &&
         "Engine must be shut down explicitly before destruction");
}

void VulkanEngine::init(core::Settings settings) {
  assert(sEngineInstance == nullptr && "Engine cannot be initialised twice");
  sEngineInstance = this;

  fmt::println("Initializing Vulcanite Engine");

  mSettings = settings;

  mHandle.init(mSettings, mWindow);

  // No more VkBootstrap - you're on your own now.
  mImgui.init(mHandle, mWindow.getSdl());

  // Allocate an image to fill the window
  initDrawImage(mSettings.initialSize);

  Vfs::Providers providers;
  auto assetDir = Vfs::getExePath().parent_path() / "assets";
  fmt::println("Using asset directory {}", assetDir.c_str());
  providers.push_back(std::make_unique<Vfs::FilesystemProvider>(assetDir));
  mVfs = std::make_unique<Vfs>(std::move(providers));

  initTextures();
  initDescriptors();
  initCommands();

  fmt::println("Ready to go!");
}

void VulkanEngine::FrameData::init(VulkanHandle &handle, VulkanEngine &engine) {
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
  auto fenceInfo =
      VulkanInit::fenceCreateInfo(vk::FenceCreateFlags::BitsType::eSignaled);
  mRenderFence = handle.createFence(/*signalled=*/true);

  mSceneUniforms.allocate(handle.mAllocator);
  mSceneUniformDescriptor = engine.mGlobalDescriptorAllocator
                                .allocate<StructBuffer<interop::SceneData>>(
                                    engine.mSceneUniformDescriptorLayout);
  mSceneUniformDescriptor.write(handle.mDevice, mSceneUniforms);
}

void VulkanEngine::FrameData::destroy(VulkanHandle &handle,
                                      VulkanEngine &engine) {
  // Destroying a queue will destroy all its buffers
  handle.mDevice.destroyCommandPool(mCommandPool, nullptr);
  handle.destroySemaphore(mSwapchainSemaphore);
  handle.destroyFence(mRenderFence);
  mSceneUniforms.free(handle.mAllocator);
}

void VulkanEngine::initDrawImage(glm::uvec2 size) {
  vk::ImageUsageFlags drawImageUsage = vk::ImageUsageFlagBits::eTransferSrc |
                                       vk::ImageUsageFlagBits::eTransferDst |
                                       vk::ImageUsageFlagBits::eStorage |
                                       vk::ImageUsageFlagBits::eColorAttachment;

  mDrawImage.destroy(mHandle);
  mDepthImage.destroy(mHandle);

  mDrawImage.init(mHandle, {size.x, size.y, 1}, vk::Format::eR16G16B16A16Sfloat,
                  drawImageUsage);
  mDepthImage.init(mHandle, {size.x, size.y, 1}, vk::Format::eD32Sfloat,
                   vk::ImageUsageFlagBits::eDepthStencilAttachment);
}

void VulkanEngine::initCommands() {
  fmt::println("Initialising command buffers");

  for (auto &buffer : mFrameData) {
    buffer.init(mHandle, *this);
  }
}

void VulkanEngine::initTextures() {
  const vk::Format format = vk::Format::eR8G8B8A8Unorm;
  auto oneByOne = vk::Extent3D(1, 1, 1);
  auto usage = vk::ImageUsageFlagBits::eSampled |
               vk::ImageUsageFlagBits::eTransferDst |
               vk::ImageUsageFlagBits::eStorage;

  mWhite.init(mHandle, oneByOne, format, usage);
  mGrey.init(mHandle, oneByOne, format, usage);
  mBlack.init(mHandle, oneByOne, format, usage);

  auto white = glm::packUnorm4x8(glm::vec4(1, 1, 1, 1));
  mWhite.fill(&white, sizeof(white));
  auto grey = glm::packUnorm4x8(glm::vec4(0.66, 0.66, 0.66, 1));
  mGrey.fill(&grey, sizeof(grey));
  auto black = glm::packUnorm4x8(glm::vec4(0, 0, 0, 1));
  mBlack.fill(&black, sizeof(black));

  // Source engine missing texture or no missing texture
  const int missingTextureSize = 16;
  mMissingTexture.init(mHandle, {missingTextureSize, missingTextureSize, 1},
                       format, usage);
  auto magenta = glm::packUnorm4x8(glm::vec4(1, 0, 1, 1));
  std::array<uint32_t, missingTextureSize * missingTextureSize>
      missingTextureData;
  for (int x = 0; x < missingTextureSize; ++x) {
    for (int y = 0; y < missingTextureSize; ++y) {
      // Alternate color
      auto color = (x + y) % 2 == 0 ? magenta : black;
      missingTextureData[x + y * missingTextureSize] = color;
    }
  }
  mMissingTexture.fill(missingTextureData);

  vk::SamplerCreateInfo samplerInfo;

  samplerInfo.magFilter = vk::Filter::eNearest;
  samplerInfo.minFilter = vk::Filter::eNearest;
  check(mHandle.mDevice.createSampler(&samplerInfo, nullptr,
                                      &mDefaultNearestSampler));

  samplerInfo.magFilter = vk::Filter::eLinear;
  samplerInfo.minFilter = vk::Filter::eLinear;
  check(mHandle.mDevice.createSampler(&samplerInfo, nullptr,
                                      &mDefaultLinearSampler));
}

void VulkanEngine::initDescriptors() {
  // Allocate a descriptor pool to hold images that compute shaders may write to
  std::array<DescriptorAllocator::PoolSizeRatio, 3> sizes = {{
      {vk::DescriptorType::eStorageImage, 1},
      {vk::DescriptorType::eUniformBuffer, 1},
      {vk::DescriptorType::eCombinedImageSampler, 1},
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
  mDrawImageDescriptors.write(mHandle.mDevice, {mDrawImage.getView()});

  DescriptorLayoutBuilder uniformBuilder;
  uniformBuilder.addBinding(0, vk::DescriptorType::eUniformBuffer);
  mSceneUniformDescriptorLayout =
      uniformBuilder.build(mHandle.mDevice, vk::ShaderStageFlagBits::eVertex);

  ShaderStage stage("gradient.comp.spv",
                    vk::ShaderStageFlags::BitsType::eCompute, "main");
  mGradientShader.link(mDrawImageDescriptorLayout, stage,
                       sizeof(interop::GradientPushConstants));

  ShaderStage triangleStage("triangle.vert.spv",
                            vk::ShaderStageFlags::BitsType::eVertex, "main");
  ShaderStage fragmentStage("triangle.frag.spv",
                            vk::ShaderStageFlags::BitsType::eFragment, "main");

  DescriptorLayoutBuilder samplerBuilder;
  samplerBuilder.addBinding(0, vk::DescriptorType::eCombinedImageSampler);
  mTextureDescriptorLayout =
      samplerBuilder.build(mHandle.mDevice, vk::ShaderStageFlagBits::eFragment);
  mTextureDescriptors =
      mGlobalDescriptorAllocator.allocate<ImageSamplerDescriptor>(
          mTextureDescriptorLayout);
  mTextureDescriptors.write(
      mHandle.mDevice, {mMissingTexture.getView(), mDefaultNearestSampler});

  mTrianglePipeline =
      Pipeline::Builder()
          .setShaders(triangleStage, fragmentStage)
          .setInputTopology(vk::PrimitiveTopology::eTriangleList)
          .setPolygonMode(vk::PolygonMode::eFill)
          .setCullMode(vk::CullModeFlagBits::eNone, vk::FrontFace::eClockwise)
          .addInputAttribute({0, 0, Pipeline::Builder::InputFloat4,
                              offsetof(interop::Vertex, position)})
          .addInputAttribute({1, 0, Pipeline::Builder::InputFloat4,
                              offsetof(interop::Vertex, color)})
          // .addInputAttribute({2, 0, Pipeline::Builder::InputFloat3,
          //                     offsetof(interop::Vertex, normal)})
          .addInputAttribute({3, 0, Pipeline::Builder::InputFloat2,
                              offsetof(interop::Vertex, uv)})
          .setPushConstantSize(vk::ShaderStageFlagBits::eVertex,
                               sizeof(interop::VertexPushConstants))
          .disableMultisampling()
          .enableAlphaBlend()
          .addDescriptorSetLayout(mSceneUniformDescriptorLayout)
          .addDescriptorSetLayout(mTextureDescriptorLayout)
          .enableDepth(true, vk::CompareOp::eGreaterOrEqual)
          .setDepthFormat(mDepthImage.getFormat())
          .setColorAttachFormat(mDrawImage.getFormat())
          .build(mHandle.mDevice);

  auto meshes = Mesh::load(mHandle, "third_party/basicmesh.glb");

  for (auto &mesh : meshes) {
    auto ent = mEcs.createEntity();
    mEcs.addComponent(ent, ecs::Transform{});
    mEcs.addComponent(ent, ecs::Renderable{mesh});
  }
}

void VulkanEngine::run() {
  while (!mWindow.quitRequested()) {
    ImGui::NewFrame();
    mWindow.update();
    ImGui_ImplVulkan_NewFrame();

    if (ImGui::Begin("Background")) {
      // ImGui::SliderInt("Mesh Index", &mFileMeshIndex, 0,
      //                  mFileMeshes.size() - 1);
    }

    ImGui::End();
    ImGui::Render();
    if (mWindow.resized()) {
      mHandle.resizeSwapchain(mWindow.getSize());
      initDrawImage(mWindow.getSize());
      mDrawImageDescriptors.write(mHandle.mDevice, {mDrawImage.getView()});
    }
    draw();
  }
}

void VulkanEngine::drawBackground(vk::CommandBuffer cmd) {
  cmd.bindPipeline(vk::PipelineBindPoint::eCompute, mGradientShader.mPipeline);
  cmd.bindDescriptorSets(
      vk::PipelineBindPoint::eCompute, mGradientShader.mLayout, /*firstSet=*/0,
      /*descriptorSetCount=*/1, &mDrawImageDescriptors.getSet(),
      /*dynamicOffsetCount=*/0,
      /*pDynamicOffsets=*/nullptr);

  cmd.pushConstants(mGradientShader.mLayout,
                    vk::ShaderStageFlags::BitsType::eCompute, 0,
                    sizeof(interop::GradientPushConstants), &mPushConstants);

  const int workgroupSize = 16;
  vkCmdDispatch(cmd, std::ceil(mWindow.getSize().x / workgroupSize) + 1,
                std::ceil(mWindow.getSize().y / workgroupSize) + 1, 1);
}

void VulkanEngine::drawScene(vk::CommandBuffer cmd) {
  auto &frameData = getCurrentFrame();
  vk::RenderingAttachmentInfo colorAttach = VulkanInit::renderAttachInfo(
      mDrawImage.getView(), nullptr, vk::ImageLayout::eColorAttachmentOptimal);
  vk::ClearValue depthClear = {.depthStencil = {.depth = 0.0f}};
  auto depthAttach =
      VulkanInit::renderAttachInfo(mDepthImage.getView(), &depthClear,
                                   vk::ImageLayout::eDepthAttachmentOptimal);
  vk::RenderingInfo renderInfo = VulkanInit::renderInfo(
      cast(mWindow.getSize()), &colorAttach, &depthAttach);

  cmd.beginRendering(&renderInfo);
  cmd.bindPipeline(vk::PipelineBindPoint::eGraphics,
                   mTrianglePipeline.getPipeline());
  std::array<vk::DescriptorSet, 2> descriptorSets = {
      frameData.mSceneUniformDescriptor.getSet(), mTextureDescriptors.getSet()};
  cmd.bindDescriptorSets(
      vk::PipelineBindPoint::eGraphics, mTrianglePipeline.getLayout(),
      /*firstSet=*/0, /*descriptorSetCount=*/2, descriptorSets.data(),
      /*dynamicOffsetCount=*/0, /*pDynamicOffsets=*/nullptr);

  auto identity = glm::identity<glm::mat4>();
  auto model = glm::rotate(identity, glm::radians((float)mFrameNumber * .25f),
                           glm::vec3{0, 1, 0});
  auto view = glm::translate(identity, glm::vec3{0, 0, -5});
  auto projection =
      glm::perspective(glm::radians(70.0f),
                       (float)mWindow.getSize().x / (float)mWindow.getSize().y,
                       // Inverse near and far to improve quality, and avoid
                       // wasting precision near the camera
                       /*zNear=*/10000.0f, /*zFar=*/.1f);
  frameData.mSceneUniforms.data()->viewProjection = projection * view * model;

  // cmd.pushConstants(
  //     mTrianglePipeline.getLayout(), vk::ShaderStageFlags::BitsType::eVertex,
  //     0, sizeof(interop::VertexPushConstants), &mVertexPushConstants);

  vk::Viewport viewport = {
      .x = 0,
      .y = 0,
      .width = static_cast<float>(mWindow.getSize().x),
      .height = static_cast<float>(mWindow.getSize().y),
      .minDepth = 0.0f,
      .maxDepth = 1.0f,
  };
  cmd.setViewport(0, 1, &viewport);

  vk::Rect2D scissor = {
      .offset = {0, 0},
      .extent = cast(mWindow.getSize()),
  };
  cmd.setScissor(0, 1, &scissor);

  mEcs.forEach<ecs::Transform, ecs::Renderable>(
      [&](ecs::EntityId id, ecs::Transform &transform,
          ecs::Renderable &renderable) {
        // TODO: Use transform
        // TODO: Renderables should use shared resources
        cmd.bindIndexBuffer(renderable.mMesh.mIndexBuffer.getBuffer(), 0,
                            vk::IndexType::eUint32);
        vk::DeviceSize offset = 0;
        cmd.bindVertexBuffers(0, 1, &renderable.mMesh.mVertexBuffer.getBuffer(),
                              &offset);
        cmd.drawIndexed(/*indexCount=*/renderable.mMesh.mIndices.size(),
                        /*instanceCount=*/1, /*firstIndex=*/0,
                        /*vertexOffset=*/0, /*firstInstance=*/0);
      });

  cmd.endRendering();
}

void VulkanEngine::draw() {
  auto &frame = getCurrentFrame();
  auto timeout = chronoToVulkan(std::chrono::seconds(1));

  // Wait for the previous frame to finish
  check(mHandle.mDevice.waitForFences(1, &frame.mRenderFence, true, timeout));
  check(mHandle.mDevice.resetFences(1, &frame.mRenderFence));

  // Request a buffer to draw to
  uint32_t swapchainImageIndex;
  check(vkAcquireNextImageKHR(mHandle.mDevice, mHandle.mSwapchain, timeout,
                              frame.mSwapchainSemaphore, nullptr,
                              &swapchainImageIndex));
  auto &swapchainEntry = mHandle.mSwapchainEntries[swapchainImageIndex];

  auto cmd = frame.mCommandBuffer;
  // We're certain the command buffer is not in use, prepare for recording
  check(vkResetCommandBuffer(cmd, 0));
  // We won't be submitting the buffer multiple times in a row, let Vulkan know
  // Drivers may be able to get a small speed boost
  auto beginInfo = VulkanInit::commandBufferBeginInfo(
      vk::CommandBufferUsageFlags::BitsType::eOneTimeSubmit);
  check(cmd.begin(&beginInfo));

  // Make the draw image writable, we don't care about destroying previous
  // data
  ImageHelpers::transitionImage(cmd, mDrawImage.getImage(),
                                vk::ImageLayout::eUndefined,
                                vk::ImageLayout::eGeneral);
  ImageHelpers::transitionImage(cmd, mDepthImage.getImage(),
                                vk::ImageLayout::eUndefined,
                                vk::ImageLayout::eDepthAttachmentOptimal);

  drawBackground(cmd);

  ImageHelpers::transitionImage(cmd, mDrawImage.getImage(),
                                vk::ImageLayout::eGeneral,
                                vk::ImageLayout::eColorAttachmentOptimal);

  drawScene(cmd);

  // Make the draw image readable again
  ImageHelpers::transitionImage(cmd, mDrawImage.getImage(),
                                vk::ImageLayout::eColorAttachmentOptimal,
                                vk::ImageLayout::eTransferSrcOptimal);

  // Copy draw image to the swapchain
  ImageHelpers::transitionImage(cmd, swapchainEntry.image,
                                vk::ImageLayout::eUndefined,
                                vk::ImageLayout::eTransferDstOptimal);
  Image::copyToSwapchainImage(cmd, mDrawImage, swapchainEntry.image,
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
  // Present the image once render is complete
  check(mHandle.mGraphicsQueue.presentKHR(&presentInfo));
  mFrameNumber++;
}

void VulkanEngine::shutdown() {
  assert(sEngineInstance == this && "Engine must exist to be shut down");

  fmt::println("Vulcanite shutting down. Goodbye!");

  // Let the GPU finish its work
  vkDeviceWaitIdle(mHandle.mDevice);
  for (auto &frameData : mFrameData) {
    frameData.destroy(mHandle, *this);
  }
  mImgui.destroy(mHandle);
  mEcs.forEach<ecs::Renderable>(
      [&](ecs::EntityId id, ecs::Renderable &r) { r.mMesh.free(mHandle); });

  mGradientShader.free();
  mTrianglePipeline.destroy(mHandle.mDevice);
  mGlobalDescriptorAllocator.destroy();
  // This will also destroy all descriptor sets allocated by it
  vkDestroyDescriptorSetLayout(mHandle.mDevice, mDrawImageDescriptorLayout,
                               nullptr);
  mHandle.mDevice.destroyDescriptorSetLayout(mSceneUniformDescriptorLayout,
                                             nullptr);
  mHandle.mDevice.destroyDescriptorSetLayout(mTextureDescriptorLayout, nullptr);

  mDrawImage.destroy(mHandle);
  mDepthImage.destroy(mHandle);
  mWhite.destroy(mHandle);
  mGrey.destroy(mHandle);
  mBlack.destroy(mHandle);
  mMissingTexture.destroy(mHandle);

  mHandle.mDevice.destroySampler(mDefaultNearestSampler, nullptr);
  mHandle.mDevice.destroySampler(mDefaultLinearSampler, nullptr);

  mHandle.shutdown();

  sEngineInstance = nullptr;
}

} // namespace selwonk::vulkan
