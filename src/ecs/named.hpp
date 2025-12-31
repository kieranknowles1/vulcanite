#pragma once

#include "../core/fixedstring.hpp"
#include "component.hpp"

namespace selwonk::ecs {
// A short name for an entity, to distinguish from others in debug messages
struct Named {
  const static constexpr ComponentType Type = ComponentType::Named;
  const static constexpr char* Name = "Named";

  // Fit in exactly 64 bytes
  core::FixedString<char, 62> mName;
};
} // namespace selwonk::ecs
