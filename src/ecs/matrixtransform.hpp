#pragma once

#include "component.hpp"

#include <glm/glm.hpp>

namespace selwonk::ecs {
struct MatrixTransform {
  const static constexpr ComponentType Type = ComponentType::MatrixTransform;
  const static constexpr char* Name = "MatrixTransform";
  // TODO: Having both transform types is awful, pick one. GLTF uses both >:(
  // But converting a scene graph to a matrix transform is not trivial and I
  // can't be bothered with the maths at 1am
  // Pretend this doesn't exist as much as you can
  glm::mat4 mTransform;
};
} // namespace selwonk::ecs
