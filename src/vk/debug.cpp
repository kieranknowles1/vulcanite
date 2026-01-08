#include "debug.hpp"

#include "../../assets/shaders/debug.h"

#include "vulkan/vulkan.hpp"
#include "vulkanengine.hpp"
#include "vulkanhandle.hpp"
#include <glm/ext/matrix_transform.hpp>
#include <glm/gtc/constants.hpp>

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
  mLineCount = 0;
}

void Debug::draw(vk::CommandBuffer cmd, const glm::mat4 &viewProjection) {
  cmd.bindPipeline(vk::PipelineBindPoint::eGraphics, mPipeline.getPipeline());
  interop::DebugPushConstants pushConstants = {
      .viewProjection = viewProjection,
      .vertexBuffer = mBuffer.getBuffer().getDeviceAddress()};

  cmd.pushConstants(mPipeline.getLayout(), vk::ShaderStageFlagBits::eVertex, 0,
                    sizeof(interop::DebugPushConstants), &pushConstants);

  cmd.draw(mLineCount * 2, /*instanceCount=*/1,
           /*firstVertex=*/0,
           /*firstInstance=*/0);
}

void Debug::drawLine(const DebugLine &line) {
  mBuffer.allocate(interop::DebugLine{.position = glm::vec4(line.start, 1.0f),
                                      .color = line.color});
  mBuffer.allocate(interop::DebugLine{.position = glm::vec4(line.end, 1.0f),
                                      .color = line.color});

  mLineCount++;
}

void Debug::drawBox(glm::vec3 origin, glm::vec3 halfExtent, glm::vec4 color) {
  glm::vec3 corner = origin - halfExtent;
  glm::vec3 size = halfExtent * 2.0f;

  const static constexpr std::array<std::pair<glm::vec3, glm::vec3>, 12>
      unitCube{{
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

  for (const auto &line : unitCube) {
    auto start = corner + (line.first * size);
    auto end = corner + (line.second * size);
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

void Debug::drawSphere(glm::vec3 origin, float radius, glm::vec4 color,
                       int resolution) {
  auto angle = [&](int index) {
    // int capI = index % resolution;
    return glm::two_pi<float>() * ((float)index / (float)resolution);
  };
  auto rotate = [](glm::vec2 point, float angle) {
    auto cosTheta = std::cos(angle);
    auto sinTheta = std::sin(angle);

    return glm::vec2((point.x * cosTheta - point.y * sinTheta),
                     (point.y * cosTheta + point.x * sinTheta));
  };

  auto sphereRadiusAt = [&](float sliceHeight) {
    // Apply Pythagoras to the triangle between the sphere's radius, the slice
    // height, and the slice radius
    return std::sqrt(radius * radius - sliceHeight * sliceHeight);
  };

  for (int x = 0; x < resolution; x++) {
    float fraction = static_cast<float>(x) / resolution;
    float height = ((radius * 2) * fraction) - radius;

    glm::vec2 point(sphereRadiusAt(height), 0);

    for (int y = 0; y < resolution; y++) {
      auto currAng = angle(y);
      auto prevAng = angle(y - 1);
      // TODO: Display rings on horizontal plane
      drawLine({
          glm::vec3(rotate(point, currAng), height) + origin,
          glm::vec3(rotate(point, prevAng), height) + origin,
          color,
      });
    }
  }

  // TODO: Connect rings
}

} // namespace selwonk::vulkan
