#pragma once

#include "../times.hpp"

namespace selwonk::ecs {
class Registry;

class System {
public:
  virtual void update(ecs::Registry& registry, Duration dt) = 0;
  // Does this system forbid the use of barriers after it and why?
  // Returned view must be static
  virtual std::optional<std::string_view> blocksBarriers() const noexcept {
    return std::nullopt;
  }
  virtual ~System() = default;
};
} // namespace selwonk::ecs
