#include "debug.hpp"

#include "../../assets/shaders/debug.h"

#include "vulkan/vulkan.hpp"
#include "vulkanengine.hpp"
#include "vulkanhandle.hpp"
#include <glm/ext/matrix_transform.hpp>

namespace selwonk::vulkan {

Debug::Debug()
    // Write directly to VRAM
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
          .disableDepth()
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

  cmd.draw(mCount * 2, /*instanceCount=*/1,
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

void Debug::drawBox(glm::vec3 origin, glm::vec3 halfExtent, glm::vec4 color) {
  glm::vec3 corner = origin - halfExtent;
  glm::vec3 extents = halfExtent * 2.0f;

  const std::array<std::pair<glm::vec3, glm::vec3>, 12> lines{{
      // Top
      {{1, 1, 1}, {0, 1, 1}},
      {{1, 1, 0}, {0, 1, 0}},
      {{1, 1, 1}, {1, 1, 0}},
      {{0, 1, 1}, {0, 1, 0}},

      // Bottom
      {{1, 0, 1}, {0, 0, 1}},
      {{1, 0, 0}, {0, 0, 0}},
      {{1, 0, 1}, {1, 0, 0}},
      {{0, 0, 1}, {0, 0, 0}},

      // Sides
      {{1, 1, 1}, {1, 0, 1}},
      {{0, 1, 1}, {0, 0, 1}},
      {{0, 1, 0}, {0, 0, 0}},
      {{1, 1, 0}, {1, 0, 0}},
  }};

  for (auto line : lines) {
    auto start = corner + (line.first * extents);
    auto end = corner + (line.second * extents);
    drawLine({start, end, color});
  }
}

void Debug::drawAxisLines(glm::vec3 position, float length) {
  // X
  drawLine({position, position + glm::vec3(length, 0, 0), Red});
  // Y
  drawLine({position, position + glm::vec3(0, length, 0), Green});
  // Z
  drawLine({position, position + glm::vec3(0, 0, length), Blue});
}

} // namespace selwonk::vulkan
