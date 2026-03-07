#include "rendersystem.hpp"

#include "../ecs/registry.hpp"
#include "debug.hpp"
#include "frustum.hpp"
#include "vulkan/vulkan.hpp"
#include "vulkanengine.hpp"
#include "vulkaninit.hpp"
#include <vulkan/vk_enum_string_helper.h>

namespace selwonk::vulkan {
RenderSystem::RenderSystem(VulkanEngine& engine) : mEngine(engine) {}

void RenderSystem::update(ecs::Registry& registry, Duration dt) {
  mEngine.prepareRendering();

  auto& frameData = mEngine.getCurrentFrame();
  frameData.mFrameData.reset();

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
                         &mEngine.mDrawImageDescriptors,
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
  vk::Extent2D extent = {
      camera.mDrawTarget->getExtent().width,
      camera.mDrawTarget->getExtent().height,
  };

  vk::RenderingAttachmentInfo colorAttach =
      VulkanInit::renderAttachInfo(camera.mDrawTarget->getView(), nullptr,
                                   vk::ImageLayout::eColorAttachmentOptimal);
  vk::ClearValue depthClear = {.depthStencil = {.depth = 0.0f}};
  auto depthAttach =
      VulkanInit::renderAttachInfo(camera.mDepthTarget->getView(), &depthClear,
                                   vk::ImageLayout::eDepthAttachmentOptimal);
  vk::RenderingInfo renderInfo =
      VulkanInit::renderInfo(extent, &colorAttach, &depthAttach);

  cmd.beginRendering(&renderInfo);
  cmd.bindPipeline(vk::PipelineBindPoint::eGraphics,
                   mEngine.mOpaquePipeline.getPipeline());

  auto staticDescriptors = mEngine.getStaticDescriptors(frameData);
  cmd.bindDescriptorSets(
      vk::PipelineBindPoint::eGraphics, mEngine.mOpaquePipeline.getLayout(),
      /*firstSet=*/0, /*descriptorSetCount=*/staticDescriptors.size(),
      staticDescriptors.data(),
      /*dynamicOffsetCount=*/0, /*pDynamicOffsets=*/nullptr);

  auto view = glm::inverse(cameraTransform.modelMatrix());
  auto projection = camera.getMatrix();
  auto viewProj = projection * view;
  frameData.mSceneUniforms.data()->viewProjection = viewProj;

  vk::Viewport viewport = {
      .x = 0,
      .y = 0,
      .width = static_cast<float>(extent.width),
      .height = static_cast<float>(extent.height),
      .minDepth = 0.0f,
      .maxDepth = 1.0f,
  };
  cmd.setViewport(0, 1, &viewport);

  vk::Rect2D scissor = {
      .offset = {0, 0},
      .extent = extent,
  };
  cmd.setScissor(0, 1, &scissor);

  Frustum clip;
  clip.fillFromMatrix(viewProj);

  int drawn = 0;
  int total = 0;

  // TODO: Make this as bindless as possible
  auto drawDataOffset = frameData.mFrameData.offset();
  uint32_t drawCount = 0;
  mEngine.mEcs.forEach<ecs::Transform, ecs::Renderable>(
      [&](ecs::EntityRef entity, ecs::Transform& transform,
          ecs::Renderable& renderable) {
        auto modelMatrix = transform.modelMatrix();

        total++;
        if (!clip.inFrustum(modelMatrix, renderable.mMesh->mBounds)) {
          return;
        }
        drawn++;

        for (auto& surface : renderable.mMesh->mSurfaces) {
          interop::VertexInstanceData drawData = {
              .drawData =
                  {
                      .vertexCount = surface.mIndexCount,
                      .instanceCount = 1,
                      .firstVertex = surface.mIndexOffset,
                      .firstInstance = drawCount,
                  },
              .modelMatrix = modelMatrix,
              .materialData = surface.mMaterial->mData,
              .indexBufferIndex = renderable.mMesh->mIndexBufferIndex.value(),
              .textureIndex = surface.mMaterial->mTexture.value(),
              .samplerIndex = surface.mMaterial->mSampler.value(),
              .vertexIndex = renderable.mMesh->mVertexIndex.value(),
          };
          frameData.mFrameData.allocate(drawData);
          drawCount++;
        }
      });
  cmd.drawIndirect(frameData.mFrameDataBuffer.getBuffer(), drawDataOffset,
                   drawCount,
                   /*stride=*/sizeof(interop::VertexInstanceData));

  core::Profiler::get().getExtraMetrics().drawnRenderable = drawn;
  core::Profiler::get().getExtraMetrics().totalRenderable = total;

  Debug::get().draw(cmd, frameData.mSceneUniformDescriptor);
  Debug::get().reset();

  cmd.endRendering();
}

void RenderSystem::draw(const ecs::Transform& cameraTransform,
                        const ecs::Camera& camera) {
  auto& frame = mEngine.getCurrentFrame();
  auto cmd = frame.mCommandBuffer;

  // Make the draw image writable, we don't care about destroying previous
  // data
  Image::transition(cmd, camera.mDrawTarget->getImage(),
                    vk::ImageLayout::eUndefined, vk::ImageLayout::eGeneral);
  Image::transition(cmd, camera.mDepthTarget->getImage(),
                    vk::ImageLayout::eUndefined,
                    vk::ImageLayout::eDepthAttachmentOptimal);

  drawBackground(cmd);

  Image::transition(cmd, camera.mDrawTarget->getImage(),
                    vk::ImageLayout::eGeneral,
                    vk::ImageLayout::eColorAttachmentOptimal);

  drawScene(cameraTransform, camera);

  // Make the draw image readable again
  Image::transition(cmd, camera.mDrawTarget->getImage(),
                    vk::ImageLayout::eColorAttachmentOptimal,
                    vk::ImageLayout::eTransferSrcOptimal);
}

} // namespace selwonk::vulkan
