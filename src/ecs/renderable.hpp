#pragma once

#include "component.hpp"

#include "../vk/mesh.hpp"

namespace selwonk::ecs {
struct Renderable {
  const static constexpr ComponentType Type = ComponentType::Renderable;

  vulkan::Mesh mMesh;
};
} // namespace selwonk::ecs
