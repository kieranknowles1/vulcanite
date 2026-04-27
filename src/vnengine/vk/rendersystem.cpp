#include "rendersystem.hpp"

#include <vncore/frustum.hpp>

#include "../ecs/registry.hpp"
#include "debug.hpp"
#include "vncore/bumpallocator.hpp"
#include "vncore/profiler.hpp"
#include "vulkan/vulkan.hpp"
#include "vulkanengine.hpp"
#include "vulkaninit.hpp"
#include <glm/gtx/norm.hpp>
#include <vulkan/vk_enum_string_helper.h>

namespace selwonk::vulkan {
RenderSystem::RenderSystem(VulkanEngine& engine) : mEngine(engine) {}

void RenderSystem::update(ecs::Registry& registry, core::Duration dt) {
  mEngine.prepareRendering();

  auto& frameData = mEngine.getCurrentFrame();
  frameData.mFrameData.reset();

  registry.forEach<ecs::Transform&, ecs::Camera&>(
      [&](ecs::EntityRef entity, auto transform, auto camera) {
        draw(transform, camera);
      });
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

void RenderSystem::beginRenderPipeline(vk::CommandBuffer cmd,
                                       vk::Pipeline pipeline) {
  cmd.bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline);

  auto staticDescriptors =
      mEngine.getStaticDescriptors(mEngine.getCurrentFrame());
  cmd.bindDescriptorSets(
      vk::PipelineBindPoint::eGraphics, mEngine.mOpaquePipeline.getLayout(),
      /*firstSet=*/0, /*descriptorSetCount=*/staticDescriptors.size(),
      staticDescriptors.data(),
      /*dynamicOffsetCount=*/0, /*pDynamicOffsets=*/nullptr);
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
  beginRenderPipeline(cmd, mEngine.mOpaquePipeline.getPipeline());

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

  core::Frustum clip;
  clip.fillFromMatrix(viewProj);

  int drawn = 0;
  int total = 0;
  mTransparent.clear();

  // TODO: Make this as bindless as possible
  auto drawDataOffset = frameData.mFrameData.offset();
  uint32_t drawCount = 0;
  mEngine.mEcs.forEach<ecs::Transform&, ecs::Renderable&>(
      [&](ecs::EntityRef entity, auto transform, auto renderable) {
        auto modelMatrix = transform.modelMatrix();

        total++;
        auto& mesh = mEngine.mMeshes.get(renderable.mMesh);
        if (!clip.inFrustum(modelMatrix, mesh.mBounds * modelMatrix)) {
          return;
        }
        drawn++;

        for (auto& surface : mesh.mSurfaces) {
          switch (surface.mMaterial.mPass) {
          case Material::Pass::Opaque:
            drawSurface(modelMatrix, mesh, surface, frameData.mFrameData,
                        drawCount);
            drawCount++;
            break;
          case Material::Pass::Translucent:
            float distance = glm::length2(cameraTransform.mTranslation -
                                          transform.mTranslation);
            mTransparent.push_back(TransparentDrawData{
                .cameraDistanceSquared = distance,
                .modelMatrix = modelMatrix,
                .mesh = &mesh,
                .surface = &surface,
            });

            break;
          }
        }
      });
  cmd.drawIndirect(frameData.mFrameDataBuffer.getBuffer(), drawDataOffset,
                   drawCount,
                   /*stride=*/sizeof(interop::VertexInstanceData));

  std::sort(mTransparent.rbegin(), mTransparent.rend());
  auto transparentOffset = frameData.mFrameData.offset();
  int transparentCount = mTransparent.size();
  for (int i = 0; i < transparentCount; i++) {
    auto& transparent = mTransparent[i];
    drawSurface(transparent.modelMatrix, *transparent.mesh,
                *transparent.surface, frameData.mFrameData, drawCount + i);
  }

  beginRenderPipeline(cmd, mEngine.mTranslucentPipeline.getPipeline());

  // FIXME: Light shafts are being loaded opaque
  cmd.drawIndirect(frameData.mFrameDataBuffer.getBuffer(), transparentOffset,
                   transparentCount,
                   /*stride=*/sizeof(interop::VertexInstanceData));

  core::Profiler::get().getExtraMetrics().drawnRenderable = drawn;
  core::Profiler::get().getExtraMetrics().totalRenderable = total;
  core::Profiler::get().getExtraMetrics().transparentRenderable =
      mTransparent.size();

  Debug::get().draw(cmd, frameData.mSceneUniformDescriptor);
  Debug::get().reset();

  cmd.endRendering();
}

void RenderSystem::drawSurface(const glm::mat4& modelMatrix, const Mesh& mesh,
                               const Mesh::Surface& surface,
                               core::BumpAllocator& allocator,
                               unsigned int index) {
  interop::VertexInstanceData drawData = {
      .drawData =
          {
              .vertexCount = surface.mIndexCount,
              .instanceCount = 1,
              .firstVertex = surface.mIndexOffset,
              .firstInstance = index,
          },
      .modelMatrix = modelMatrix,
      .materialDataIndex = surface.mMaterial.mDataIndex.value(),
      .indexBufferIndex = mesh.mIndexBufferIndex.value(),
      .textureIndex = surface.mMaterial.mTexture.value(),
      .samplerIndex = surface.mMaterial.mSampler.value(),
      .vertexIndex = mesh.mVertexIndex.value(),
  };
  allocator.allocate(drawData);
}

void RenderSystem::draw(const ecs::Transform& cameraTransform,
                        const ecs::Camera& camera) {
  auto& frame = mEngine.getCurrentFrame();
  auto cmd = frame.mCommandBuffer;
  auto& profiler = VulkanEngine::get().mProfiler;

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

  profiler.pushSection("Cull");
  drawScene(cameraTransform, camera);

  // Make the draw image readable again
  profiler.siblingSection("Prepare for present");
  Image::transition(cmd, camera.mDrawTarget->getImage(),
                    vk::ImageLayout::eColorAttachmentOptimal,
                    vk::ImageLayout::eTransferSrcOptimal);

  profiler.popSection();
}

} // namespace selwonk::vulkan
