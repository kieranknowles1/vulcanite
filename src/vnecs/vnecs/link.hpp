#pragma once

#include <array>

#include "component.hpp"
#include "entity.hpp"

namespace selwonk::ecs {
// Link to other entities
// A link can be to the entity itself
struct Link {
  enum class Type {
    // Next node for a CameraPathSystem
    Next,
    // "root" enity of the scene this was instantiated for
    SceneOrigin,
    Max,
  };
  
  const static constexpr ComponentType Type = ComponentType::Link;
  const static constexpr char* Name = "Link";
  using Store = ComponentArray<Link>;

  void set(enum Type type, EntityRef value) {
    mLinks[static_cast<int>(type)] = value;
  }
  EntityRef get(enum Type type) {
    return mLinks[static_cast<int>(type)];
  }

  std::array<EntityRef, static_cast<int>(Type::Max)> mLinks;
};
} // namespace selwonk::ecs
