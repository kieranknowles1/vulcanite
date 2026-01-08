#pragma once

#include "../core/singleton.hpp"
#include "buffer.hpp"
#include "shader.hpp"

#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

namespace selwonk::vulkan {
class Debug : core::Singleton<Debug> {
public:
  struct DebugLine {
    glm::vec3 start;
    glm::vec3 end;
    glm::vec3 color;
  };

  const static constexpr glm::vec3 Red{1, 0, 0};
  const static constexpr glm::vec3 Green{0, 1, 0};
  const static constexpr glm::vec3 Blue{0, 0, 1};

  void drawLine(const DebugLine& line);
  void drawAxisLines(glm::vec3 position);
  void drawCube(glm::vec3 origin, glm::vec3 halfExtent);
  void drawSphere(glm::vec3 origin, float radius);

private:
  Pipeline mPipeline;
  Buffer mBuffer;
  size_t mCount;
};
} // namespace selwonk::vulkan
