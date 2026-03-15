#pragma once

#include "component.hpp"

#include "../vk/mesh.hpp"
#include "vncore/handelist.hpp"

namespace selwonk::ecs {
struct Renderable {
  const static constexpr ComponentType Type = ComponentType::Renderable;
  const static constexpr char* Name = "Renderable";
  using Store = ComponentArray<Renderable>;

  core::HandleList<vulkan::Mesh>::Handle mMesh;

  const void onEcsAdd() const;
  const void onEcsRemove() const;
};
} // namespace selwonk::ecs
