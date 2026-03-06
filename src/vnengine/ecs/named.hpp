#pragma once

#include "../core/fixedstring.hpp"
#include "component.hpp"

namespace selwonk::ecs {
// A short name for an entity, to distinguish from others in debug messages
struct Named {
  const static constexpr ComponentType Type = ComponentType::Named;
  const static constexpr char* Name = "Named";
  using Store = ComponentArray<Named>;

  // Fit in exactly 64 bytes
  core::FixedString<char, 63> mName;
};
} // namespace selwonk::ecs
