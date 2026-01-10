#include "rendersystem.hpp"

#include "../ecs/registry.hpp"
#include "../times.hpp"
#include "imagehelpers.hpp"
#include "utility.hpp"
#include "vulkanengine.hpp"
#include <vulkan/vk_enum_string_helper.h>

namespace selwonk::vulkan {
RenderSystem::RenderSystem(VulkanEngine& engine) : mEngine(engine) {}

void RenderSystem::update(ecs::Registry& registry, float dt) {
  registry.forEach<ecs::Transform, ecs::Camera>(
      [&](ecs::EntityRef entity, const ecs::Transform& transform,
          const ecs::Camera& camera) { draw(transform, camera); });
}

void RenderSystem::drawBackground(vk::CommandBuffer cmd) {
  cmd.bindPipeline(vk::PipelineBindPoint::eCompute,
                   mEngine.mGradientShader.mPipeline);
  cmd.bindDescriptorSets(vk::PipelineBindPoint::eCompute,
                         mEngine.mGradientShader.mLayout, /*firstSet=*/0,
                         /*descriptorSetCount=*/1,
                         &mEngine.mDrawImageDescriptors.getSet(),
                         /*dynamicOffsetCount=*/0,
                         /*pDynamicOffsets=*/nullptr);

  cmd.pushConstants(
      mEngine.mGradientShader.mLayout, vk::ShaderStageFlags::BitsType::eCompute,
      0, sizeof(interop::GradientPushConstants), &mEngine.mPushConstants);

  const int workgroupSize = 16;
  vkCmdDispatch(cmd, std::ceil(mEngine.mWindow.getSize().x / workgroupSize) + 1,
                std::ceil(mEngine.mWindow.getSize().y / workgroupSize) + 1, 1);
}

void RenderSystem::drawScene(const ecs::Transform& cameraTransform,
                             const ecs::Camera& camera) {
  auto& frameData = mEngine.getCurrentFrame();
  auto cmd = frameData.mCommandBuffer;
  vk::RenderingAttachmentInfo colorAttach =
      VulkanInit::renderAttachInfo(camera.mDrawTarget->getView(), nullptr,
                                   vk::ImageLayout::eColorAttachmentOptimal);
  vk::ClearValue depthClear = {.depthStencil = {.depth = 0.0f}};
  auto depthAttach =
      VulkanInit::renderAttachInfo(camera.mDepthTarget->getView(), &depthClear,
                                   vk::ImageLayout::eDepthAttachmentOptimal);
  auto extent = camera.mDrawTarget->getExtent();
  vk::RenderingInfo renderInfo = VulkanInit::renderInfo(
      {extent.width, extent.height}, &colorAttach, &depthAttach);

  cmd.beginRendering(&renderInfo);
  cmd.bindPipeline(vk::PipelineBindPoint::eGraphics,
                   mEngine.mOpaquePipeline.getPipeline());

  std::array<vk::DescriptorSet, 3> staticDescriptors = {
      frameData.mSceneUniformDescriptor.getSet(),
      mEngine.mSamplerCache.getDescriptorSet(),
      mEngine.mTextureCache.getDescriptorSet(),
  };

  cmd.bindDescriptorSets(
      vk::PipelineBindPoint::eGraphics, mEngine.mOpaquePipeline.getLayout(),
      /*firstSet=*/0, /*descriptorSetCount=*/staticDescriptors.size(),
      staticDescriptors.data(),
      /*dynamicOffsetCount=*/0, /*pDynamicOffsets=*/nullptr);

  auto& transform =
      mEngine.mEcs.getComponent<ecs::Transform>(mEngine.mPlayerCamera);

  auto view = glm::inverse(transform.modelMatrix());

  auto projection = camera.getMatrix();
  frameData.mSceneUniforms.data()->viewProjection = projection * view;

  vk::Viewport viewport = {
      .x = 0,
      .y = 0,
      .width = static_cast<float>(mEngine.mWindow.getSize().x),
      .height = static_cast<float>(mEngine.mWindow.getSize().y),
      .minDepth = 0.0f,
      .maxDepth = 1.0f,
  };
  cmd.setViewport(0, 1, &viewport);

  vk::Rect2D scissor = {
      .offset = {0, 0},
      .extent = cast(mEngine.mWindow.getSize()),
  };
  cmd.setScissor(0, 1, &scissor);

  // TODO: Make this as bindless as possible
  mEngine.mEcs.forEach<ecs::Transform, ecs::Renderable>(
      [&](ecs::EntityRef entity, ecs::Transform& transform,
          ecs::Renderable& renderable) {
        for (auto& surface : renderable.mMesh->mSurfaces) {
          interop::VertexPushConstants pushConstants = {
              .modelMatrix = transform.modelMatrix(),
              .indexBuffer = renderable.mMesh->mIndexBuffer.getDeviceAddress(),
              .vertexBuffer =
                  renderable.mMesh->mVertexBuffer.getDeviceAddress(),
              .materialData = surface.mMaterial->mData.gpu,
              .textureIndex = surface.mMaterial->mTexture.value(),
              .samplerIndex = surface.mMaterial->mSampler.value(),
          };
          cmd.pushConstants(mEngine.mOpaquePipeline.getLayout(),
                            vk::ShaderStageFlagBits::eVertex |
                                vk::ShaderStageFlagBits::eFragment,
                            0, sizeof(interop::VertexPushConstants),
                            &pushConstants);

          cmd.draw(surface.mIndexCount, /*instanceCount=*/1,
                   /*firstVertex=*/surface.mIndexOffset,
                   /*firstInstance=*/0);
        }
      });
  mEngine.mDebug->draw(cmd, frameData.mSceneUniformDescriptor.getSet());
  mEngine.mDebug->reset();

  cmd.endRendering();
}

void RenderSystem::draw(const ecs::Transform& cameraTransform,
                        const ecs::Camera& camera) {
  auto& frame = mEngine.getCurrentFrame();
  auto timeout = chronoToVulkan(std::chrono::seconds(1));

  // Wait for the previous frame to finish
  mEngine.mProfiler.startSection("Await VSync");
  check(mEngine.mHandle.mDevice.waitForFences(1, &frame.mRenderFence, true,
                                              timeout));
  check(mEngine.mHandle.mDevice.resetFences(1, &frame.mRenderFence));

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
  ImageHelpers::transitionImage(cmd, camera.mDrawTarget->getImage(),
                                vk::ImageLayout::eUndefined,
                                vk::ImageLayout::eGeneral);
  ImageHelpers::transitionImage(cmd, camera.mDepthTarget->getImage(),
                                vk::ImageLayout::eUndefined,
                                vk::ImageLayout::eDepthAttachmentOptimal);

  mEngine.mProfiler.startSection("Background");
  drawBackground(cmd);

  ImageHelpers::transitionImage(cmd, camera.mDrawTarget->getImage(),
                                vk::ImageLayout::eGeneral,
                                vk::ImageLayout::eColorAttachmentOptimal);

  mEngine.mProfiler.startSection("Scene");
  drawScene(cameraTransform, camera);

  // Make the draw image readable again
  ImageHelpers::transitionImage(cmd, camera.mDrawTarget->getImage(),
                                vk::ImageLayout::eColorAttachmentOptimal,
                                vk::ImageLayout::eTransferSrcOptimal);
}

} // namespace selwonk::vulkan
