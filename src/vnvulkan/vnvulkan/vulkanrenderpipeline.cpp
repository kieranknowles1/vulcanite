#include "vulkanrenderpipeline.hpp"

#include <spdlog/spdlog.h>

#include <vncore/cvar.hpp>

#include "vulkaninit.hpp"
#include "utility.hpp"

namespace selwonk::vulkan {

core::Cvar::Int
  MaxFrameInstances("vulkan.max_frame_instances", 64 * 1024,
    "Maximum number of instances per frame",
    core::util::combineFlags(core::Cvar::Flags::InitOnly,
      core::Cvar::Flags::Unsigned));

VulkanRenderPipeline::VulkanRenderPipeline(VulkanHandle& handle, sdl::Window& window, core::ThreadPool& threadPool, core::Vfs& vfs)
  : mHandle(handle), mNativeHandles(threadPool), mWindow(window), mVfs(vfs)
{
  SPDLOG_INFO("Initialising descriptors");

  // No more VkBootstrap - you're on your own now.
  mImgui.init(handle, mWindow.getSdl());

  // Allocate a descriptor pool to hold images that compute shaders may write to
  std::array<DescriptorAllocator::PoolSizeRatio, 4> sizes = { {
      {vk::DescriptorType::eStorageImage, 1},
      {vk::DescriptorType::eUniformBuffer, 1},
      {vk::DescriptorType::eStorageBuffer,
       static_cast<float>(MaxFrameInstances.value())},
      {vk::DescriptorType::eSampledImage, 1},
  } };

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

  ShaderStage stage(vfs.get("shaders/gradient.comp.spv"),
    vk::ShaderStageFlags::BitsType::eCompute, "main");
  mGradientShader.link(mDrawImageDescriptorLayout, stage,
    sizeof(interop::GradientPushConstants));

  SPDLOG_INFO("Initialising command buffers");
  for (auto& buffer : mFrameData) {
    buffer.init(mHandle, *this);
  }

  // Changing descriptor array sizes will dirty pipelines
  auto dirtyBuffers = [this](int _) { mPipelinesDirty = true; };
  VulkanNativeHandleProvider::MaxVertexBuffers.getStore().addChange(
    dirtyBuffers);
  VulkanNativeHandleProvider::MaxTextures.getStore().addChange(dirtyBuffers);
}

void VulkanRenderPipeline::waitIdle() const {
  CHECK(mHandle.mDevice.waitIdle());
}

VulkanRenderPipeline::~VulkanRenderPipeline()
{
  // Let the GPU finish its work
  waitIdle();

  for (auto& frameData : mFrameData) {
    frameData.destroy(mHandle, *this);
  }

  mGlobalDescriptorAllocator.destroy();
  // This will also destroy all descriptor sets allocated by it
  mHandle.mDevice.destroyDescriptorSetLayout(mDrawImageDescriptorLayout,
    nullptr);
  mHandle.mDevice.destroyDescriptorSetLayout(mSceneUniformDescriptorLayout,
    nullptr);
  mHandle.mDevice.destroyDescriptorSetLayout(mInstanceDataLayout, nullptr);

  mGradientShader.free();

  mImgui.destroy(mHandle);
}

void VulkanRenderPipeline::present(const ecs::Camera& mainCamera)
{
  auto& frame = getCurrentFrame();
  auto cmd = frame.mCommandBuffer;

  // Request a buffer to draw to
  uint32_t swapchainImageIndex;
  CHECK(mHandle.mDevice.acquireNextImageKHR(
    mHandle.mSwapchain, RenderTimeout, frame.mSwapchainSemaphore,
    nullptr, &swapchainImageIndex));
  auto& swapchainEntry = mHandle.mSwapchainEntries[swapchainImageIndex];

  // Copy draw image to the swapchain
  Image::transition(cmd, swapchainEntry.image, vk::ImageLayout::eUndefined,
    vk::ImageLayout::eTransferDstOptimal);
  Image::copyToSwapchainImage(
    cmd, getNativeHandles().getNativeTextures().getTexture(mainCamera.mImages.draw),
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

  vk::PresentInfoKHR presentInfo{ .waitSemaphoreCount = 1,
                                 .pWaitSemaphores = &swapchainEntry.semaphore,
                                 .swapchainCount = 1,
                                 .pSwapchains = &mHandle.mSwapchain,
                                 .pImageIndices = &swapchainImageIndex };
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

void VulkanRenderPipeline::FrameData::init(VulkanHandle& handle, VulkanRenderPipeline& engine) {
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

void VulkanRenderPipeline::initPipelines() {
  mPipelinesDirty = false;
  ShaderStage triangleStage(mVfs.get("shaders/triangle.vert.spv"),
    vk::ShaderStageFlags::BitsType::eVertex, "main");
  ShaderStage fragmentStage(mVfs.get("shaders/triangle.frag.spv"),
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
    .setDepthFormat(VulkanRenderPipeline::DepthFormat)
    .setColorAttachFormat(VulkanRenderPipeline::DrawFormat);

  mOpaquePipeline = builder.build(mHandle.mDevice);
  mTranslucentPipeline = builder
    // Disable depth write
    .enableDepth(false, vk::CompareOp::eGreaterOrEqual)
    .enableAlphaBlend()
    .build(mHandle.mDevice);
}

void VulkanRenderPipeline::FrameData::destroy(VulkanHandle& handle,
  VulkanRenderPipeline& engine) {
  // Destroying a queue will destroy all its buffers
  handle.mDevice.destroyCommandPool(mCommandPool, nullptr);
  handle.destroySemaphore(mSwapchainSemaphore);
  handle.destroyFence(mRenderFence);
  mSceneUniforms.free(handle.mAllocator);
  mFrameDataBuffer.free(handle.mAllocator);
}

ecs::Camera::Images VulkanRenderPipeline::createDrawImage(glm::uvec2 size)
{
  vk::ImageUsageFlags drawImageUsage = vk::ImageUsageFlagBits::eTransferSrc |
    vk::ImageUsageFlagBits::eTransferDst |
    vk::ImageUsageFlagBits::eStorage |
    vk::ImageUsageFlagBits::eColorAttachment;

  vk::Extent3D drawExtent = { size.x, size.y, 1 };

  Image draw;
  draw.allocate(drawExtent, DrawFormat, drawImageUsage, "ImgDraw");
  Image depth;
  depth.allocate(drawExtent, DepthFormat,
    vk::ImageUsageFlagBits::eDepthStencilAttachment, "ImgDepth");

  // Ensure descriptors are in place for the background
  DescriptorAllocator::writeImage(mDrawImageDescriptors, draw.getView(), 0,
    vk::ImageLayout::eGeneral,
    vk::DescriptorType::eStorageImage);

  return {
      .draw = getNativeHandles().getNativeTextures().insert(draw),
      .depth = getNativeHandles().getNativeTextures().insert(depth),
  };
}

}