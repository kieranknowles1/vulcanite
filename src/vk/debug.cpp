#include "debug.hpp"

#include "../../assets/shaders/triangle.h"

#include "vulkan/vulkan.hpp"
#include "vulkanengine.hpp"
#include "vulkanhandle.hpp"
#include <glm/ext/matrix_transform.hpp>
#include <glm/gtc/constants.hpp>

namespace selwonk::vulkan {

Debug::Debug()
    // Write directly to VRAM
    : mBuffer(MaxDebugLines * sizeof(interop::Vertex) * 2,
              vk::BufferUsageFlagBits::eStorageBuffer) {
  ShaderStage triangleStage("debug.vert.spv",
                            vk::ShaderStageFlags::BitsType::eVertex, "main");
  ShaderStage fragmentStage("debug.frag.spv",
                            vk::ShaderStageFlags::BitsType::eFragment, "main");
  auto& engine = VulkanEngine::get();
  auto builder =
      Pipeline::Builder()
          .setShaders(triangleStage, fragmentStage)
          .setInputTopology(vk::PrimitiveTopology::eLineList)
          .setPolygonMode(vk::PolygonMode::eFill)
          .setPushConstantSize(vk::ShaderStageFlagBits::eVertex,
                               sizeof(interop::VertexPushConstants))
          .addDescriptorSetLayout(engine.mSceneUniformDescriptorLayout)
          // TODO: Function for adding all needed layouts
          .addDescriptorSetLayout(engine.mSamplerCache.getDescriptorLayout())
          .addDescriptorSetLayout(engine.mTextureManager.getDescriptorLayout())
          .addDescriptorSetLayout(engine.mBufferLayout)
          .disableMultisampling()
          .disableBlending()
          .disableDepth()
          .setDepthFormat(engine.getCamera().mDepthTarget->getFormat())
          .setColorAttachFormat(engine.getCamera().mDrawTarget->getFormat());
  mPipeline = builder.build(VulkanHandle::get().mDevice);

  ShaderStage solidTriangleStage(
      "triangle.vert.spv", vk::ShaderStageFlags::BitsType::eVertex, "main");
  mSolidPipeline = builder.setShaders(solidTriangleStage, fragmentStage)
                       .setInputTopology(vk::PrimitiveTopology::eTriangleList)
                       .build(VulkanHandle::get().mDevice);
}

void Debug::reset() {
  mBuffer.reset();
  mDebugMeshes.clear();
  mLineCount = 0;
}

void Debug::draw(vk::CommandBuffer cmd, vk::DescriptorSet drawDescriptors) {
  cmd.bindPipeline(vk::PipelineBindPoint::eGraphics,
                   mSolidPipeline.getPipeline());
  cmd.bindDescriptorSets(vk::PipelineBindPoint::eGraphics,
                         mPipeline.getLayout(),
                         /*firstSet=*/0,
                         /*descriptorSetCount=*/1, &drawDescriptors,
                         /*dynamicOffsetCount=*/0,
                         /*pDynamicOffsets=*/nullptr);

  for (auto& mesh : mDebugMeshes) {
    for (auto& surface : mesh.mesh.mSurfaces) {
      interop::VertexPushConstants meshPushConstants = {
          .modelMatrix = mesh.transform,
          .indexBuffer = mesh.mesh.mIndexBuffer.getDeviceAddress(),
          .materialData = surface.mMaterial->mData.gpu,
          .vertexIndex = mesh.mesh.mVertexIndex,
      };
      cmd.pushConstants(mPipeline.getLayout(), vk::ShaderStageFlagBits::eVertex,
                        0, sizeof(interop::VertexPushConstants),
                        &meshPushConstants);
      cmd.draw(surface.mIndexCount, /*instanceCount=*/1,
               /*firstVertex=*/surface.mIndexOffset,
               /*firstInstance=*/0);
    }
  }

  cmd.bindPipeline(vk::PipelineBindPoint::eGraphics, mPipeline.getPipeline());
  interop::VertexPushConstants pushConstants = {
      .modelMatrix = glm::identity<glm::mat4>(),
      // TODO: Properly bind vertex buffer, probably allocate with engine's
      // allocator once it's there
      .vertexIndex = 0,
  };

  cmd.pushConstants(mPipeline.getLayout(), vk::ShaderStageFlagBits::eVertex, 0,
                    sizeof(interop::VertexPushConstants), &pushConstants);

  // TODO: Function on engine for this
  auto& mEngine = VulkanEngine::get();
  auto& frameData = mEngine.getCurrentFrame();
  std::array<vk::DescriptorSet, 4> staticDescriptors = {
      frameData.mSceneUniformDescriptor.getSet(),
      mEngine.mSamplerCache.getDescriptorSet(),
      mEngine.mTextureManager.getDescriptorSet(),
      mEngine.mBufferSet,
  };

  cmd.bindDescriptorSets(
      vk::PipelineBindPoint::eGraphics, mPipeline.getLayout(),
      /*firstSet=*/0,
      /*descriptorSetCount=*/staticDescriptors.size(), staticDescriptors.data(),
      /*dynamicOffsetCount=*/0,
      /*pDynamicOffsets=*/nullptr);

  cmd.draw(mLineCount * 2, /*instanceCount=*/1,
           /*firstVertex=*/0,
           /*firstInstance=*/0);
}

void Debug::drawLine(const DebugLine& line) {
  mBuffer.allocate(interop::Vertex{.position = glm::vec4(line.start, 1.0f),
                                   .color = line.color});
  mBuffer.allocate(interop::Vertex{.position = glm::vec4(line.end, 1.0f),
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

  for (const auto& line : unitCube) {
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

  auto ringPos = [&](int index) {
    glm::vec2 point(radius, 0);
    auto currAng = angle(index);
    auto prevAng = angle(index - 1);
    return std::make_pair(rotate(point, currAng), rotate(point, prevAng));
  };

  for (int i = 0; i < resolution; i++) {
    auto positions = ringPos(i);

    // vertical x-aligned
    drawLine({
        glm::vec3(0, positions.first.x, positions.first.y) + origin,
        glm::vec3(0, positions.second.x, positions.second.y) + origin,
        color,
    });

    // horizontal
    drawLine({
        glm::vec3(positions.first.x, 0, positions.first.y) + origin,
        glm::vec3(positions.second.x, 0, positions.second.y) + origin,
        color,
    });

    // vertical z-aligned
    drawLine({
        glm::vec3(positions.first.x, positions.first.y, 0) + origin,
        glm::vec3(positions.second.x, positions.second.y, 0) + origin,
        color,
    });
  }
}

} // namespace selwonk::vulkan
