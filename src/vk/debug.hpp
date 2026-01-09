#pragma once

#include "../core/singleton.hpp"
#include "bumpallocator.hpp"
#include "shader.hpp"
#include "vulkan/vulkan.hpp"

#include <glm/fwd.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

namespace selwonk::vulkan {
class Debug : public core::Singleton<Debug> {
public:
  const static constexpr size_t MaxDebugLines = 1024 * 1024;

  struct DebugLine {
    glm::vec3 start;
    glm::vec3 end;
    glm::vec4 color;
  };

  const static constexpr glm::vec4 Red{1, 0, 0, 1};
  const static constexpr glm::vec4 Green{0, 1, 0, 1};
  const static constexpr glm::vec4 Blue{0, 0, 1, 1};

  Debug();
  ~Debug();

  void reset();
  void draw(vk::CommandBuffer cmd, vk::DescriptorSet drawDescriptors);

  // Draw a line, must be called every frame
  void drawLine(const DebugLine& line);
  void drawAxisLines(glm::vec3 position, float length = 1.0f);
  void drawBox(glm::vec3 origin, glm::vec3 halfExtent, glm::vec4 color);
  void drawSphere(glm::vec3 origin, float radius, glm::vec4 color,
                  int resolution = 16);

private:
  Pipeline mPipeline;
  BumpAllocator mBuffer;
  size_t mLineCount = 0;
};
} // namespace selwonk::vulkan
