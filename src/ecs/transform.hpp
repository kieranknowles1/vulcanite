#pragma once

#include "component.hpp"
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

namespace selwonk::ecs {
struct Transform {
  const static constexpr ComponentType Type = ComponentType::Transform;
  const static constexpr char* Name = "Transform";

  glm::mat4 modelMatrix() {
    return glm::translate(glm::mat4(1.0f), mPosition) * rotationMatrix() *
           glm::scale(glm::mat4(1.0f), mScale);
  }

  glm::mat4 rotationMatrix() { return glm::mat4_cast(mRotation); }

  glm::vec3 mPosition = glm::vec3(0.0f);
  glm::quat mRotation = glm::identity<glm::quat>();
  glm::vec3 mScale = glm::vec3(1.0f);
};
} // namespace selwonk::ecs
