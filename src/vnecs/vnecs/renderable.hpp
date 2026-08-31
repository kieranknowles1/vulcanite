#pragma once

#include <vnassets/mesh.hpp>

#include "component.hpp"

namespace selwonk::ecs {
struct Renderable {
  const static constexpr ComponentType Type = ComponentType::Renderable;
  const static constexpr char* Name = "Renderable";
  using Store = ComponentArray<Renderable>;

  assets::MeshData::Handle mMesh;

  const void onEcsAdd() const;
  const void onEcsRemove() const;
};
} // namespace selwonk::ecs
