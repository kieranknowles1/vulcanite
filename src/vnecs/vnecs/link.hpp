#pragma once

#include "component.hpp"
#include "entity.hpp"

namespace selwonk::ecs {
// Link to another entity in a chain
struct Link {
  const static constexpr ComponentType Type = ComponentType::Link;
  const static constexpr char* Name = "Link";
  using Store = ComponentArray<Link>;

  EntityRef mNext;
};
} // namespace selwonk::ecs
