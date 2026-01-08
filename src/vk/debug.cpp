#include "debug.hpp"

#include "../../assets/shaders/triangle.h"

#include "vulkan/vulkan.hpp"
#include "vulkanengine.hpp"
#include "vulkanhandle.hpp"
#include <glm/ext/matrix_transform.hpp>

namespace selwonk::vulkan {

// TODO: Dedicated debug shader, could take DebugLine directly
Debug::Debug()
    : mBuffer(MaxDebugLines * sizeof(interop::Vertex) * 2,
              vk::BufferUsageFlagBits::eShaderDeviceAddress),
      mIndexBuffer(MaxDebugLines * sizeof(unsigned int) * 2,
                   vk::BufferUsageFlagBits::eShaderDeviceAddress) {
  ShaderStage triangleStage("triangle.vert.spv",
                            vk::ShaderStageFlags::BitsType::eVertex, "main");
  ShaderStage fragmentStage("triangle.frag.spv",
                            vk::ShaderStageFlags::BitsType::eFragment, "main");
  auto &engine = VulkanEngine::get();
  mPipeline =
      Pipeline::Builder()
          .setShaders(triangleStage, fragmentStage)
          .setInputTopology(vk::PrimitiveTopology::eLineList)
          .setPolygonMode(vk::PolygonMode::eFill)
          .setCullMode(vk::CullModeFlagBits::eBack,
                       vk::FrontFace::eCounterClockwise)
          .setPushConstantSize(vk::ShaderStageFlagBits::eVertex |
                                   vk::ShaderStageFlagBits::eFragment,
                               sizeof(interop::VertexPushConstants))
          .disableMultisampling()
          .disableBlending()
          .addDescriptorSetLayout(engine.mSceneUniformDescriptorLayout)
          .addDescriptorSetLayout(
              engine.getSamplerCache().getDescriptorLayout())
          .addDescriptorSetLayout(
              engine.getTextureCache().getDescriptorLayout())
          .enableDepth(false, vk::CompareOp::eAlways)
          .setDepthFormat(engine.getCamera().mDepthTarget->getFormat())
          .setColorAttachFormat(engine.getCamera().mDrawTarget->getFormat())
          .build(VulkanHandle::get().mDevice);
}

void Debug::reset() {
  mBuffer.reset();
  mIndexBuffer.reset();
  mCount = 0;
}

void Debug::draw(vk::CommandBuffer cmd, vk::DescriptorSet mUniforms) {
  auto &engine = VulkanEngine::get();
  cmd.bindPipeline(vk::PipelineBindPoint::eGraphics, mPipeline.getPipeline());
  interop::VertexPushConstants pushConstants = {
      .modelMatrix = glm::identity<glm::mat4>(),
      .indexBuffer = mIndexBuffer.getBuffer().getDeviceAddress(),
      .vertexBuffer = mBuffer.getBuffer().getDeviceAddress(),
      .materialData = engine.mDefaultMaterial.mData.gpu,
      .textureIndex = engine.mDefaultMaterial.mTexture.value(),
      .samplerIndex = engine.mDefaultMaterial.mSampler.value()};

  std::array<vk::DescriptorSet, 1> staticDescriptors = {
      mUniforms,
  };

  cmd.bindDescriptorSets(
      vk::PipelineBindPoint::eGraphics, mPipeline.getLayout(),
      /*firstSet=*/0, /*descriptorSetCount=*/staticDescriptors.size(),
      staticDescriptors.data(),
      /*dynamicOffsetCount=*/0, /*pDynamicOffsets=*/nullptr);

  cmd.pushConstants(mPipeline.getLayout(),
                    vk::ShaderStageFlagBits::eVertex |
                        vk::ShaderStageFlagBits::eFragment,
                    0, sizeof(interop::VertexPushConstants), &pushConstants);

  cmd.draw(mCount * 3, /*instanceCount=*/1,
           /*firstVertex=*/0,
           /*firstInstance=*/0);
}

void Debug::drawLine(const DebugLine &line) {
  mBuffer.allocate(
      interop::Vertex{.position = line.start, .color = line.color});
  mBuffer.allocate(interop::Vertex{.position = line.end, .color = line.color});
  int indexStart = mCount * 2;
  mIndexBuffer.allocate(indexStart);
  mIndexBuffer.allocate(indexStart + 1);

  mCount++;
}

void Debug::drawAxisLines(glm::vec3 position) {
  // X
  drawLine({position, position + glm::vec3(5, 0, 0), Red});
  // Y
  drawLine({position, position + glm::vec3(0, 5, 0), Green});
  // Z
  drawLine({position, position + glm::vec3(0, 0, 5), Blue});
}

} // namespace selwonk::vulkan
