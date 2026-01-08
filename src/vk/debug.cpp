#include "debug.hpp"

#include "../../assets/shaders/debug.h"

#include "vulkan/vulkan.hpp"
#include "vulkanengine.hpp"
#include "vulkanhandle.hpp"
#include <glm/ext/matrix_transform.hpp>

namespace selwonk::vulkan {

// TODO: Dedicated debug shader, could take DebugLine directly
Debug::Debug()
    : mBuffer(MaxDebugLines * sizeof(interop::DebugLine) * 2,
              vk::BufferUsageFlagBits::eShaderDeviceAddress) {
  ShaderStage triangleStage("debug.vert.spv",
                            vk::ShaderStageFlags::BitsType::eVertex, "main");
  ShaderStage fragmentStage("debug.frag.spv",
                            vk::ShaderStageFlags::BitsType::eFragment, "main");
  auto &engine = VulkanEngine::get();
  mPipeline =
      Pipeline::Builder()
          .setShaders(triangleStage, fragmentStage)
          .setInputTopology(vk::PrimitiveTopology::eLineList)
          .setPolygonMode(vk::PolygonMode::eFill)
          .setPushConstantSize(vk::ShaderStageFlagBits::eVertex,
                               sizeof(interop::DebugPushConstants))
          .disableMultisampling()
          .disableBlending()
          .enableDepth(false, vk::CompareOp::eAlways)
          .setDepthFormat(engine.getCamera().mDepthTarget->getFormat())
          .setColorAttachFormat(engine.getCamera().mDrawTarget->getFormat())
          .build(VulkanHandle::get().mDevice);
}

Debug::~Debug() { mPipeline.destroy(VulkanHandle::get().mDevice); }

void Debug::reset() {
  mBuffer.reset();
  mCount = 0;
}

void Debug::draw(vk::CommandBuffer cmd, const glm::mat4 &viewProjection) {
  cmd.bindPipeline(vk::PipelineBindPoint::eGraphics, mPipeline.getPipeline());
  interop::DebugPushConstants pushConstants = {
      .viewProjection = viewProjection,
      .vertexBuffer = mBuffer.getBuffer().getDeviceAddress()};

  cmd.pushConstants(mPipeline.getLayout(), vk::ShaderStageFlagBits::eVertex, 0,
                    sizeof(interop::DebugPushConstants), &pushConstants);

  cmd.draw(mCount * 4, /*instanceCount=*/1,
           /*firstVertex=*/0,
           /*firstInstance=*/0);
}

void Debug::drawLine(const DebugLine &line) {
  mBuffer.allocate(interop::DebugLine{.position = glm::vec4(line.start, 1.0f),
                                      .color = line.color});
  mBuffer.allocate(interop::DebugLine{.position = glm::vec4(line.end, 1.0f),
                                      .color = line.color});

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
