#pragma once

#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

#include <vncore/singleton.hpp>

#include "mesh.hpp"

namespace selwonk::assets {
class Debug : public core::Singleton<Debug> {
public:
  struct DebugLine {
    glm::vec3 start;
    glm::vec3 end;
    glm::vec4 color;
  };
  struct DebugMesh {
    glm::mat4 transform;
    // TODO: Use handles here
    const Mesh& mesh;
  };

  const static constexpr glm::vec4 Red{1, 0, 0, 1};
  const static constexpr glm::vec4 Green{0, 1, 0, 1};
  const static constexpr glm::vec4 Blue{0, 0, 1, 1};

  void reset() {
    mLines.clear();
    mMeshes.clear();
  }

  // Draw a line, must be called every frame
  void drawLine(const DebugLine& line) {
    mLines.emplace_back(line);
  }
  void drawAxisLines(glm::vec3 position, float length = 1.0f);
  void drawBox(glm::vec3 origin, glm::vec3 halfExtent, glm::vec4 color);
  void drawSphere(glm::vec3 origin, float radius, glm::vec4 color,
                  int resolution = 16);
  // TODO: Allow setting colour of mesh
  void drawMesh(const glm::mat4& transform, const Mesh& mesh) {
    mMeshes.emplace_back(transform, mesh);
  }

  std::span<const DebugLine> getLines() const { return mLines; }
  std::span<const DebugMesh> getMeshes() const { return mMeshes; }

private:
  std::vector<DebugLine> mLines;
  std::vector<DebugMesh> mMeshes;
};
} // namespace selwonk::vulkan
