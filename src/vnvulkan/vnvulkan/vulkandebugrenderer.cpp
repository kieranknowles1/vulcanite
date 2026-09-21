#include "vulkandebugrenderer.hpp"

#include <glm/ext/matrix_transform.hpp>

#include "../../assets/shaders/triangle.h"

#include "vulkan/vulkan.hpp"
#include "vulkanrenderpipeline.hpp"

namespace selwonk::vulkan {

VulkanDebugRenderer::VulkanDebugRenderer() {
  // Write directly to VRAM
  auto& vtxBuffers = VulkanRenderPipeline::get().getNativeHandles().getNativeVertexes();
  mBuffer = vtxBuffers.allocate(DebugBufferSize, Buffer::Usage::DebugLines,
                                "DebugLines");
  auto& buffer = vtxBuffers.getBuffer(mBuffer);
  mAllocator = core::BumpAllocator(buffer.getAllocationInfo().pMappedData,
                                   DebugBufferSize);
}

VulkanDebugRenderer::~VulkanDebugRenderer() { VulkanRenderPipeline::get().getNativeHandles().decRef(mBuffer); }

void VulkanDebugRenderer::initPipelines() {
  auto& pipeline = VulkanRenderPipeline::get();
  auto& vfs = pipeline.mVfs;

  ShaderStage triangleStage(vfs.get("shaders/debug.vert.spv"),
                            vk::ShaderStageFlags::BitsType::eVertex, "main");
  ShaderStage fragmentStage(vfs.get("shaders/debug.frag.spv"),
                            vk::ShaderStageFlags::BitsType::eFragment, "main");
  ShaderStage solidTriangleStage(vfs.get("shaders/triangle.vert.spv"),
                                 vk::ShaderStageFlags::BitsType::eVertex,
                                 "main");

  auto layouts = pipeline.getDescriptorLayouts();
  auto builder = Pipeline::Builder()
                     .setShaders(triangleStage, fragmentStage)
                     .setInputTopology(vk::PrimitiveTopology::eLineList)
                     .setPolygonMode(vk::PolygonMode::eFill)
                     .setDescriptorLayouts(std::span(layouts))
                     .disableMultisampling()
                     .disableBlending()
                     .disableDepth()
                     .setDepthFormat(VulkanRenderPipeline::DepthFormat)
                     .setColorAttachFormat(VulkanRenderPipeline::DrawFormat);
  mPipeline = builder.build(VulkanHandle::get().mDevice);

  mSolidPipeline = builder.setShaders(solidTriangleStage, fragmentStage)
                       .setInputTopology(vk::PrimitiveTopology::eTriangleList)
                       .build(VulkanHandle::get().mDevice);
}

void VulkanDebugRenderer::draw(vk::CommandBuffer cmd, vk::DescriptorSet drawDescriptors, const assets::Debug& debugData) {
  auto& pipeline = VulkanRenderPipeline::get();
  auto& frameData = pipeline.getCurrentFrame();
  auto staticDescriptors = pipeline.getStaticDescriptors(frameData);

  cmd.bindPipeline(vk::PipelineBindPoint::eGraphics,
                   mSolidPipeline.getPipeline());
  cmd.bindDescriptorSets(vk::PipelineBindPoint::eGraphics,
                         mPipeline.getLayout(),
                         /*firstSet=*/0,
                         /*descriptorSetCount=*/1, &drawDescriptors,
                         /*dynamicOffsetCount=*/0,
                         /*pDynamicOffsets=*/nullptr);

  uint32_t meshOffset = frameData.mFrameData.offset();
  uint32_t indexOffset = meshOffset / sizeof(interop::VertexInstanceData);
  uint32_t meshCount = 0;

  for (auto& mesh : debugData.getMeshes()) {
    for (auto& surface : mesh.mesh.mSurfaces) {
      interop::VertexInstanceData drawData = {
          .drawData =
              {
                  .vertexCount = surface.mIndexCount,
                  .instanceCount = 1,
                  .firstVertex = surface.mIndexOffset,
                  .firstInstance = indexOffset + meshCount,
              },
          .modelMatrix = mesh.transform,
          .materialDataIndex = surface.mMaterial.mDataIndex.value(),
          .indexBufferIndex = mesh.mesh.mIndexBufferIndex.value(),
          .textureIndex = surface.mMaterial.mTexture.value(),
          .samplerIndex = surface.mMaterial.mSampler.value(),
          .vertexIndex = mesh.mesh.mVertexIndex.value(),
      };
      frameData.mFrameData.allocate(drawData);
      meshCount++;
    }
  }
  cmd.drawIndirect(frameData.mFrameDataBuffer.getBuffer(), meshOffset,
                    meshCount, sizeof(interop::VertexInstanceData));

  // TODO: Draw in chunks. Buffer lives on the GPU. Reuse is not safe as commands depend on it
  mAllocator.reset();
  for (auto& line : debugData.getLines()) {
    mAllocator.allocate(interop::Vertex{ .position = glm::vec4(line.start, 1.0f),
                                    .color = line.color });
    mAllocator.allocate(interop::Vertex{ .position = glm::vec4(line.end, 1.0f),
                                        .color = line.color });
  }

  cmd.bindPipeline(vk::PipelineBindPoint::eGraphics, mPipeline.getPipeline());
  uint32_t lineOffset = frameData.mFrameData.offset();
  interop::VertexInstanceData drawData = {
      .drawData =
          {
              .vertexCount = (unsigned int)debugData.getLines().size() * 2,
              .instanceCount = 1,
              .firstVertex = 0,
              .firstInstance = meshCount + indexOffset,
          },
      .modelMatrix = glm::identity<glm::mat4>(),
      // TODO: Properly bind vertex buffer, probably allocate with engine's
      // allocator once it's there
      .vertexIndex = mBuffer.value(),
  };
  frameData.mFrameData.allocate(drawData);

  cmd.bindDescriptorSets(
      vk::PipelineBindPoint::eGraphics, mPipeline.getLayout(),
      /*firstSet=*/0,
      /*descriptorSetCount=*/staticDescriptors.size(), staticDescriptors.data(),
      /*dynamicOffsetCount=*/0,
      /*pDynamicOffsets=*/nullptr);

  cmd.drawIndirect(frameData.mFrameDataBuffer.getBuffer(), lineOffset, 1,
                   sizeof(interop::VertexInstanceData));
}

} // namespace selwonk::vulkan
