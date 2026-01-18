#pragma once

#include "../core/bumpallocator.hpp"
#include "../core/singleton.hpp"
#include "mesh.hpp"
#include "shader.hpp"
#include "vulkan/vulkan.hpp"

#include <glm/fwd.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

namespace selwonk::vulkan {
class Debug : public core::Singleton<Debug> {
public:
  const static constexpr size_t MaxDebugLines = 1024 * 1024;
  const static constexpr size_t DebugBufferSize =
      MaxDebugLines * sizeof(interop::Vertex) * 2;

  struct DebugLine {
    glm::vec3 start;
    glm::vec3 end;
    glm::vec4 color;
  };
  struct DebugMesh {
    glm::mat4 transform;
    const Mesh& mesh;
  };

  const static constexpr glm::vec4 Red{1, 0, 0, 1};
  const static constexpr glm::vec4 Green{0, 1, 0, 1};
  const static constexpr glm::vec4 Blue{0, 0, 1, 1};

  Debug();
  ~Debug();

  void reset();
  void draw(vk::CommandBuffer cmd, vk::DescriptorSet drawDescriptors);
  
  void initPipelines();

  // Draw a line, must be called every frame
  void drawLine(const DebugLine& line);
  void drawAxisLines(glm::vec3 position, float length = 1.0f);
  void drawBox(glm::vec3 origin, glm::vec3 halfExtent, glm::vec4 color);
  void drawSphere(glm::vec3 origin, float radius, glm::vec4 color,
                  int resolution = 16);
  void drawMesh(const glm::mat4& transform, const Mesh& mesh) {
    mDebugMeshes.emplace_back(transform, mesh);
  }

private:
  Pipeline mPipeline;
  Pipeline mSolidPipeline;
  std::vector<DebugMesh> mDebugMeshes;

  // TODO: Does this need to be frame-level data?
  BufferMap::Handle mBuffer;
  std::unique_ptr<core::BumpAllocator> mAllocator;
  size_t mLineCount = 0;
};
} // namespace selwonk::vulkan
