#include "rendersystem.hpp"

#include <vncore/frustum.hpp>

#include <vnecs/registry.hpp>
#include "vncore/bumpallocator.hpp"
#include "vncore/profiler.hpp"
#include "vulkan/vulkan.hpp"
#include <vnvulkan/vulkaninit.hpp>
#include <glm/gtx/norm.hpp>
#include <vnvulkan/utility.hpp>
#include "vulkanengine.hpp"

namespace selwonk::vulkan {
RenderSystem::RenderSystem(VulkanRenderPipeline& pipeline) : mPipeline(pipeline) {}

void RenderSystem::update(ecs::Registry& registry, core::Duration dt) {
  prepareRendering();

  auto& frameData = mPipeline.getCurrentFrame();
  frameData.mFrameData.reset();

  registry.forEach<const ecs::Transform&, const ecs::Camera&>(
      [&](ecs::EntityRef entity, auto transform, auto camera) {
        draw(transform, camera);
      });
}

VulkanRenderPipeline::FrameData& RenderSystem::prepareRendering() {
  // TODO: This assumes one render system (probably always the case)
  auto& frame = mPipeline.getCurrentFrame();
  auto cmd = frame.mCommandBuffer;

  // Wait for the previous frame to finish
  CHECK(VulkanHandle::get().mDevice.waitForFences(1, &frame.mRenderFence, true,
    VulkanRenderPipeline::RenderTimeout));
  CHECK(VulkanHandle::get().mDevice.resetFences(1, &frame.mRenderFence));

  // We're certain the command buffer is not in use, prepare for recording
  CHECK(cmd.reset(vk::CommandBufferResetFlags{}));
  // We won't be submitting the buffer multiple times in a row, let Vulkan know
  // Drivers may be able to get a small speed boost
  auto beginInfo = VulkanInit::commandBufferBeginInfo(
    vk::CommandBufferUsageFlags::BitsType::eOneTimeSubmit);
  CHECK(cmd.begin(&beginInfo));
  return frame;
}

void RenderSystem::drawBackground(vk::CommandBuffer cmd) {
  cmd.bindPipeline(vk::PipelineBindPoint::eCompute,
                   mPipeline.mGradientShader.mPipeline);
  cmd.bindDescriptorSets(vk::PipelineBindPoint::eCompute,
                         mPipeline.mGradientShader.mLayout, /*firstSet=*/0,
                         /*descriptorSetCount=*/1,
                         &mPipeline.mDrawImageDescriptors,
                         /*dynamicOffsetCount=*/0,
                         /*pDynamicOffsets=*/nullptr);

  cmd.pushConstants(
      mPipeline.mGradientShader.mLayout, vk::ShaderStageFlags::BitsType::eCompute,
      0, sizeof(interop::GradientPushConstants), &mPipeline.mPushConstants);

  const int workgroupSize = 16;
  vkCmdDispatch(cmd, std::ceil(mPipeline.getWindow().getSize().x / workgroupSize) + 1,
                std::ceil(mPipeline.getWindow().getSize().y / workgroupSize) + 1, 1);
}

void RenderSystem::beginRenderPipeline(vk::CommandBuffer cmd,
                                       vk::Pipeline pipeline) {
  cmd.bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline);

  auto staticDescriptors =
      mPipeline.getStaticDescriptors(mPipeline.getCurrentFrame());
  cmd.bindDescriptorSets(
      vk::PipelineBindPoint::eGraphics, mPipeline.getOpaquePipeline().getLayout(),
      /*firstSet=*/0, /*descriptorSetCount=*/staticDescriptors.size(),
      staticDescriptors.data(),
      /*dynamicOffsetCount=*/0, /*pDynamicOffsets=*/nullptr);
}

void RenderSystem::drawScene(const ecs::Transform& cameraTransform,
                             const ecs::Camera& camera) {
  auto& frameData = mPipeline.getCurrentFrame();
  auto cmd = frameData.mCommandBuffer;
  auto& draw = mPipeline.getNativeHandles().getNativeTextures().getTexture(camera.mImages.draw);
  auto& depth = mPipeline.getNativeHandles().getNativeTextures().getTexture(camera.mImages.depth);
  vk::Extent2D extent = {camera.mSize.x, camera.mSize.y};

  vk::RenderingAttachmentInfo colorAttach = VulkanInit::renderAttachInfo(
      draw.getView(), nullptr, vk::ImageLayout::eColorAttachmentOptimal);
  vk::ClearValue depthClear = {.depthStencil = {.depth = 0.0f}};
  auto depthAttach = VulkanInit::renderAttachInfo(
      depth.getView(), &depthClear, vk::ImageLayout::eDepthAttachmentOptimal);
  vk::RenderingInfo renderInfo =
      VulkanInit::renderInfo(extent, &colorAttach, &depthAttach);

  cmd.beginRendering(&renderInfo);
  beginRenderPipeline(cmd, mPipeline.getOpaquePipeline().getPipeline());

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
  VulkanEngine::get().mEcs.forEach<const ecs::Transform&, const ecs::Renderable&>(
      [&](ecs::EntityRef entity, auto transform, auto renderable) {
        auto modelMatrix = transform.modelMatrix();

        total++;
        auto& mesh = mPipeline.getNativeHandles().getNativeMeshes().get(renderable.mMesh);
        if (!clip.inFrustum(modelMatrix, mesh.mBounds * modelMatrix)) {
          return;
        }
        drawn++;

        for (auto& surface : mesh.mSurfaces) {
          switch (surface.mMaterial.mPass) {
          case assets::Material::Pass::Opaque:
            drawSurface(modelMatrix, mesh, surface, frameData.mFrameData,
                        drawCount);
            drawCount++;
            break;
          case assets::Material::Pass::Translucent:
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

  beginRenderPipeline(cmd, mPipeline.getTranslucentPipeline().getPipeline());

  // FIXME: Light shafts are being loaded opaque
  cmd.drawIndirect(frameData.mFrameDataBuffer.getBuffer(), transparentOffset,
                   transparentCount,
                   /*stride=*/sizeof(interop::VertexInstanceData));

  core::Profiler::get().getExtraMetrics().drawnRenderable = drawn;
  core::Profiler::get().getExtraMetrics().totalRenderable = total;
  core::Profiler::get().getExtraMetrics().transparentRenderable =
      mTransparent.size();

  VulkanEngine::get().mProfiler.siblingSection("Debug Draw");
  mDebugRenderer.draw(cmd, frameData.mSceneUniformDescriptor, assets::Debug::get());
  assets::Debug::get().reset();

  cmd.endRendering();
}

void RenderSystem::drawSurface(const glm::mat4& modelMatrix, const assets::Mesh& mesh,
                               const assets::MeshData::Surface& surface,
                               core::BumpAllocator& allocator,
                               unsigned int index) {
  interop::VertexInstanceData drawData = {
      .drawData =
          {
              .vertexCount = surface.mIndexCount,
              // TODO: Could we use instancing here? What would that even mean?
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
  auto& frame = mPipeline.getCurrentFrame();
  auto cmd = frame.mCommandBuffer;
  auto& profiler = VulkanEngine::get().mProfiler;

  // Make the draw image writable, we don't care about destroying previous
  // data
  auto& draw = mPipeline.getNativeHandles().getNativeTextures().getTexture(camera.mImages.draw);
  auto& depth = mPipeline.getNativeHandles().getNativeTextures().getTexture(camera.mImages.depth);
  Image::transition(cmd, draw.getImage(), vk::ImageLayout::eUndefined,
                    vk::ImageLayout::eGeneral);
  Image::transition(cmd, depth.getImage(), vk::ImageLayout::eUndefined,
                    vk::ImageLayout::eDepthAttachmentOptimal);

  drawBackground(cmd);

  Image::transition(cmd, draw.getImage(), vk::ImageLayout::eGeneral,
                    vk::ImageLayout::eColorAttachmentOptimal);

  profiler.pushSection("Cull");
  drawScene(cameraTransform, camera);

  // Make the draw image readable again
  profiler.siblingSection("Prepare for present");
  Image::transition(cmd, draw.getImage(),
                    vk::ImageLayout::eColorAttachmentOptimal,
                    vk::ImageLayout::eTransferSrcOptimal);

  profiler.popSection();
}

} // namespace selwonk::vulkan
