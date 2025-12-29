#pragma once

#include "component.hpp"
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

namespace selwonk::ecs {
struct Transform {
  const static constexpr ComponentType Type = ComponentType::Transform;

  glm::vec3 mPosition;
  glm::quat mRotation;
  glm::vec3 mScale;
};
} // namespace selwonk::ecs
